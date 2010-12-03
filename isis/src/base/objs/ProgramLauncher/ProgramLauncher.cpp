/**
 * @file
 * $Revision: 1.22 $
 * $Date: 2010/06/29 23:42:18 $
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are
 *   public domain. See individual third-party library and package descriptions
 *   for intellectual property information, user agreements, and related
 *   information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or
 *   implied, is made by the USGS as to the accuracy and functioning of such
 *   software and related material nor shall the fact of distribution
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */
#include "ProgramLauncher.h"

#include <iostream>
#include <sstream>
#include <sys/wait.h>

#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>

#include "Application.h"
#include "Filename.h"
#include "iException.h"
#include "iString.h"

using namespace std;

namespace Isis {
  /**
   * Executes the Isis program with the given arguments. This will handle logs,
   *   GUI updates, and similar tasks. Please use this even when there is no
   *   instance of Isis::Application, so long as the thing you are running has
   *   an Isis::Application (do not use this for qview, qnet, qtie, etc for
   *   now).
   *
   * Please do not specify -pid.
   *
   * @param programName The Isis program name to be run (i.e. catlab, cubeatt)
   * @param parameters The arguments to give to the program that is being run
   */
  void ProgramLauncher::RunIsisProgram(iString programName,
                                       iString parameters) {
    Filename program(programName);
    Filename isisExecutableFilename("$ISISROOT/bin/" + program.Name());
    bool isIsisProgram = false;

    if(isisExecutableFilename.Exists()) {
      isIsisProgram = true;
      program = isisExecutableFilename;
    }

    iString command = program.Expanded() + " " + parameters +
        " -pid=" + iString(getpid());

    if(!isIsisProgram) {
      iString msg = "Program [" + programName + "] does not appear to be a "
          "valid Isis 3 program";
      throw iException::Message(iException::System, msg, _FILEINFO_);
    }

    iString serverName = "isis_" + Application::UserName() +
        "_" + iString(getpid());

    QLocalServer server;
    server.listen(serverName);

    QProcess childProcess;
    childProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    childProcess.start(command);
    childProcess.waitForStarted();

    bool connected = false;

    while(!connected && childProcess.state() != QProcess::NotRunning) {
      // Give time for the process to connect to us or for it to finish
      connected = server.waitForNewConnection(900);
      childProcess.waitForFinished(100);
    }

    if(!connected) {
      iString msg = "Isis child process failed to communicate with parent";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }

    QLocalSocket *childSocket = server.nextPendingConnection();

    // Don't return until we're done running this program
    while(childSocket->state() != QLocalSocket::UnconnectedState) {
      bool insideCode = true;
      bool messageDone = false;

      iString code;
      iString message;
      QByteArray lineData;

      if(childSocket->waitForReadyRead(1000)) {
        lineData = childSocket->read(1024 * 1024 * 1024);

        for(int i = 0; i < lineData.size(); i++) {
          if(insideCode) {
            if(lineData[i] != (char)27) {
              code += lineData[i];
            }
            else {
              insideCode = false;
            }
          }
          else {
            if(lineData[i] != (char)27) {
              message += lineData[i];
            }
            else {
              messageDone = true;
              insideCode = true;
              i ++; // skip \n that should always exist here
            }
          }

          if(messageDone) {
            ProcessIsisMessageFromChild(code, message);
            messageDone = false;
          }
        }
      }
    }

    childProcess.waitForFinished();

    if(childProcess.exitCode() != 0) {
      iString msg = "Running Isis program [" + programName + "] failed with "
                    "return status [" + iString(childProcess.exitCode()) + "]";
      throw iException::Message(iException::System, msg, _FILEINFO_);
    }
  }


  /**
   * This interprets a message sent along the pipe from a child process to us
   *   (the parent).
   *
   * @param code The text code of the message - this is used to determine what
   *             the message contains.
   * @param msg The data sent along with a code. This is a string, number,
   *            PvlGroup, Pvl, etc... really anything. It depends on the code
   *            parameter.
   */
  void ProgramLauncher::ProcessIsisMessageFromChild(iString code, iString msg) {
    if(code == "PROGRESSTEXT") {
      if(!iApp) return; // cannot process this kind of message from qisis yet
      iApp->UpdateProgress(msg, true);
    }
    else if(code == "PROGRESS") {
      if(!iApp) return; // cannot process this kind of message from qisis yet
      iApp->UpdateProgress((int)msg, true);
    }
    else if(code == "LOG") {
      if(!iApp) return; // cannot process this kind of message from qisis yet
      stringstream msgStream;
      msgStream << msg;
      Pvl logPvl;
      msgStream >> logPvl;

      if(logPvl.Groups() == 1 &&
         logPvl.Keywords() == 0 &&
         logPvl.Objects() == 0) {
        iApp->Log(logPvl.Group(0));
      }
    }
    else if(code == "GUILOG") {
      if(!iApp) return; // cannot process this kind of message from qisis yet
      iApp->GuiLog(msg);
    }
    else if(code == "ERROR") {
      stringstream msgStream;
      msgStream << msg;
      Pvl errorPvl;
      msgStream >> errorPvl;

      for(int i = 0; i < errorPvl.Groups(); i++) {
        PvlGroup &g = errorPvl.Group(i);
        std::string eclass = g["Class"];
        std::string emsg = g["Message"];
        int ecode = g["Code"];
        std::string efile = g["File"];
        int eline = g["Line"];

        iException::Message((iException::errType)ecode,
                              emsg, efile.c_str(), eline);
      }
    }
  }


  /**
   * This runs arbitrary system commands. You can run programs like "qview" with
   *   this, or commands like "ls | grep *.cpp > out.txt". Please do not use
   *   this for Isis programs not in qisis.
   *
   * Example: qview should use RunIsisProgram to run camstats.
   *          camstats should use RunSystemCommand to run qview.
   *
   * @param fullCommand A string containing the command formatted like what
   *                    you would type in a terminal
   */
  void ProgramLauncher::RunSystemCommand(iString fullCommand) {
    int status = system(fullCommand.c_str());

    if(status != 0) {
      iString msg = "Executing command [" + fullCommand +
                    "] failed with return status [" + iString(status) + "]";
      throw Isis::iException::Message(iException::Programmer, msg, _FILEINFO_);
    }
  }
}  //end namespace isis


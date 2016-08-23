#include <QByteArray>
#include <QDebug>
#include <QDataStream>
#include <QFile>
#include <QIODevice>
#include <QString>
#include <QXmlInputSource>
#include <QXmlStreamWriter>

#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "Angle.h"
#include "BundleControlPoint.h"
#include "BundleImage.h"
#include "BundleMeasure.h"
#include "BundleObservation.h"
#include "BundleObservationSolveSettings.h"
#include "BundleObservationVector.h"
#include "BundleSettings.h"
#include "BundleTargetBody.h"
#include "Camera.h"
#include "ControlPoint.h"
#include "ControlMeasure.h"
#include "Distance.h"
#include "FileName.h"
#include "IException.h"
#include "Latitude.h"
#include "LinearAlgebra.h"
#include "Longitude.h"
#include "Preference.h"
#include "PvlObject.h"
#include "SpecialPixel.h"
#include "Spice.h"
#include "SurfacePoint.h"
#include "Target.h"
#include "XmlStackedHandlerReader.h"

using namespace std;
using namespace Isis;

void printBundleMeasure(BundleMeasure &);

/**
 * This class is used to test loading objects from xml files.
 * 
 * @author 2014 Jeannie Backer
 *
 * @internal
 *   @history 2014-12-11 Jeannie Backer - Added test for BundleControlPoint.
 *                           Updated truth file, improved overall test coverage.
 *   @history 2016-08-10 Jeannie Backer - Replaced boost vector with Isis::LinearAlgebra::Vector.
 *                           References #4163.
 *   @history 2016-08-18 Jeannie Backer - Removed references to BundleSettings solve method.
 *                           References #4162.
 */



namespace Isis {
  class XmlHandlerTester : public BundleObservationSolveSettings {
    public:
      XmlHandlerTester(Project *project, XmlStackedHandlerReader *reader, FileName xmlFile)
          : BundleObservationSolveSettings(project, reader) {

        QString xmlPath(xmlFile.expanded());
        QFile file(xmlPath);

        if (!file.open(QFile::ReadOnly) ) {
          throw IException(IException::Io,
                           QString("Unable to open xml file, [%1],  with read access").arg(xmlPath),
                           _FILEINFO_);
        }

        QXmlInputSource xmlInputSource(&file);
        bool success = reader->parse(xmlInputSource);
        if (!success) {
          throw IException(IException::Unknown, 
                           QString("Failed to parse xml file, [%1]").arg(xmlPath),
                            _FILEINFO_);
        }

      }

      ~XmlHandlerTester() {
      }

  };
}



int main(int argc, char *argv[]) {
  Preference::Preferences(true);

  cout.precision(6);
  qDebug() << "Unit test for BundleUtilities...";
  qDebug() << "";

  try {
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "Testing BundleObservationSolveSettings...";
    qDebug() << "";
    // default constructor
    qDebug() << "Printing PVL group with settings from the default constructor...";
    BundleObservationSolveSettings boss;
    PvlObject pvl = boss.pvlObject("DefaultBundleObservationSolveSettings");
    cout << pvl << endl << endl;
    
    qDebug() << "Testing copy constructor...";
    BundleObservationSolveSettings copySettings(boss);
    pvl = copySettings.pvlObject("CopySettingsObject");
    cout << pvl << endl << endl;
    
    qDebug() << "Testing assignment operator to set this equal to itself...";
    boss = boss;
    pvl = boss.pvlObject("SelfAssignedSettingsObject");
    cout << pvl << endl << endl;
    
    qDebug() << "Testing assignment operator to create a new settings object...";
    BundleObservationSolveSettings assignmentOpSettings;
    assignmentOpSettings = boss;
    pvl = assignmentOpSettings.pvlObject("AssignedSettingsObject");
    cout << pvl << endl << endl;
    
    qDebug() << "Testing mutator methods...";
    qDebug() << "setInstrument(), setInstrumentPointingSettings(), setInstrumentPositionSettings()";
    boss.setInstrumentId("MyInstrumentId");
    boss.setInstrumentPointingSettings(BundleObservationSolveSettings::AnglesVelocity, true, 1, 2,
                                       false, 3.0, 4.0, 5.0);
    boss.setInstrumentPositionSettings(BundleObservationSolveSettings::PositionOnly, 6, 7,
                                       true, 800.0, 900.0, 1000.0);
    pvl = boss.pvlObject();// using MyInstrumentId as PvlObject name
    cout << pvl << endl << endl;
    
    qDebug() << "setFromPvl()";
    PvlGroup settingsGroup1("SettingsGroupId1");
    settingsGroup1 += PvlKeyword("CAMSOLVE", "NONE");
    settingsGroup1 += PvlKeyword("SPSOLVE", "NONE");
    BundleObservationSolveSettings settingsFromGroup1;
    settingsFromGroup1.setFromPvl(settingsGroup1);
    pvl = settingsFromGroup1.pvlObject("SettingsFromPvlGroup-SolveForNone");
    cout << pvl << endl << endl;
    settingsGroup1.findKeyword("CAMSOLVE").setValue("ANGLES");
    settingsGroup1.findKeyword("SPSOLVE").setValue("POSITIONS");
    BundleObservationSolveSettings settingsFromGroup2;
    settingsFromGroup2.setFromPvl(settingsGroup1);
    pvl = settingsFromGroup2.pvlObject("SettingsFromPvlGroup-SolveForAnglesPositions");
    cout << pvl << endl << endl;
    settingsGroup1.findKeyword("CAMSOLVE").setValue("VELOCITIES");
    settingsGroup1.findKeyword("SPSOLVE").setValue("VELOCITIES");
    BundleObservationSolveSettings settingsFromGroup3;
    settingsFromGroup3.setFromPvl(settingsGroup1);
    pvl = settingsFromGroup3.pvlObject("SettingsFromPvlGroup-SolveForVelocities");
    cout << pvl << endl << endl;
    settingsGroup1.findKeyword("CAMSOLVE").setValue("ACCELERATIONS");
    settingsGroup1.findKeyword("SPSOLVE").setValue("ACCELERATIONS");
    BundleObservationSolveSettings settingsFromGroup4;
    settingsFromGroup4.setFromPvl(settingsGroup1);
    pvl = settingsFromGroup4.pvlObject("SettingsFromPvlGroup-SolveForAccelerations");
    cout << pvl << endl << endl;
    
    PvlGroup settingsGroup2("SettingsGroupId2");
    settingsGroup2 += PvlKeyword("CAMSOLVE", "All");
    settingsGroup2 += PvlKeyword("TWIST", "No");
    settingsGroup2 += PvlKeyword("CKDEGREE", "4");
    settingsGroup2 += PvlKeyword("CKSOLVEDEGREE", "5");
    settingsGroup2 += PvlKeyword("OVEREXISTING", "true");
    settingsGroup2 += PvlKeyword("CAMERA_ANGLES_SIGMA", "1.0");
    settingsGroup2 += PvlKeyword("CAMERA_ANGULAR_VELOCITY_SIGMA", "-1.0");
    settingsGroup2 += PvlKeyword("CAMERA_ANGULAR_ACCELERATION_SIGMA", "3.0");
    settingsGroup2 += PvlKeyword("SPSOLVE", "All");
    settingsGroup2 += PvlKeyword("SPKDEGREE", "6");
    settingsGroup2 += PvlKeyword("SPKSOLVEDEGREE", "7");
    settingsGroup2 += PvlKeyword("OVERHERMITE", "true");
    settingsGroup2 += PvlKeyword("SPACECRAFT_POSITION_SIGMA", "8.0");
    settingsGroup2 += PvlKeyword("SPACECRAFT_VELOCITY_SIGMA", "9.0");
    settingsGroup2 += PvlKeyword("SPACECRAFT_ACCELERATION_SIGMA", "-1.0");
    boss.setFromPvl(settingsGroup2);
    pvl = boss.pvlObject("SettingsFromPvlGroup-SolveForAllCoefficients");
    cout << pvl << endl << endl;

    qDebug() << "Testing static unused enum-to-string and string-to-enum methods...";
    qDebug() << BundleObservationSolveSettings::instrumentPointingSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPointingSolveOption(
                                                                              "NOPOINTINGFACTORS"));
    qDebug() << BundleObservationSolveSettings::instrumentPointingSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPointingSolveOption(
                                                                                     "anglesonly"));
    qDebug() << BundleObservationSolveSettings::instrumentPointingSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPointingSolveOption(
                                                                              "AnglesAndVelocity"));
    qDebug() << BundleObservationSolveSettings::instrumentPointingSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPointingSolveOption(
                                                                  "AnglesVelocityAndAcceleration"));
    qDebug() << BundleObservationSolveSettings::instrumentPointingSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPointingSolveOption(
                                                                      "AllPolynomialCoefficients"));
    qDebug() << BundleObservationSolveSettings::instrumentPositionSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPositionSolveOption(
                                                                              "NOPOSITIONFACTORS"));
    qDebug() << BundleObservationSolveSettings::instrumentPositionSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPositionSolveOption(
                                                                                   "positiononly"));
    qDebug() << BundleObservationSolveSettings::instrumentPositionSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPositionSolveOption(
                                                                            "PositionAndVelocity"));
    qDebug() << BundleObservationSolveSettings::instrumentPositionSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPositionSolveOption(
                                                                "PositionVelocityAndAcceleration"));
    qDebug() << BundleObservationSolveSettings::instrumentPositionSolveOptionToString(
                BundleObservationSolveSettings::stringToInstrumentPositionSolveOption(
                                                                      "AllPolynomialCoefficients"));
    qDebug() << "";

    qDebug() << "Testing serialization...";
    QByteArray byteArray;
    QDataStream outputData(&byteArray, QIODevice::WriteOnly);
    outputData << boss;
    QDataStream inputData(byteArray);
    BundleObservationSolveSettings newBoss;
    inputData >> newBoss;
    pvl = newBoss.pvlObject("SerializedSettings");
    cout << pvl << endl;
//    QFile file("BundleObservationSolveSettingsTest.dat");
//    file.open(QIODevice::WriteOnly);
//    QDataStream out(&file);
//    out << boss;
//    file.close();
//    file.open(QIODevice::ReadOnly);
//    QDataStream in(&file);
//    in >> newBoss;
    qDebug() << "";

    qDebug() << "Testing XML: write XML from BundleObservationSolveSettings object...";
    // write xml 
    FileName xmlFile("./BundleObservationSolveSettings.xml");
    QString xmlPath = xmlFile.expanded();
    QFile qXmlFile(xmlPath);
  
    // For test coverage, we need to write/read BundleObservationSolveSettings objects
    // with 0,1,2,3 apriori sigmas and an empty xml
    if (!qXmlFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
      throw IException(IException::Io,
                       QString("Unable to open xml file, [%1],  with write access").arg(xmlPath),
                       _FILEINFO_);
    }
    QXmlStreamWriter writer(&qXmlFile);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    Project *project = NULL;
    settingsFromGroup1.save(writer, project);
    writer.writeEndDocument();
    qXmlFile.close();
    // read xml    
    qDebug() << "Testing XML: read XML to BundleObservationSolveSettings object...";
    XmlStackedHandlerReader reader;
    XmlHandlerTester bsFromXml1(project, &reader, xmlFile);
    pvl = bsFromXml1.pvlObject("FromXml-1");
    cout << pvl << endl << endl;

    if (!qXmlFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
      throw IException(IException::Io,
                       QString("Unable to open xml file, [%1],  with write access").arg(xmlPath),
                       _FILEINFO_);
    }
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    settingsFromGroup2.save(writer, project);
    writer.writeEndDocument();
    qXmlFile.close();
    // read xml    
    qDebug() << "Testing XML: read XML to BundleObservationSolveSettings object...";
    XmlHandlerTester bsFromXml2(project, &reader, xmlFile);
    pvl = bsFromXml2.pvlObject("FromXml-2");
    cout << pvl << endl << endl;

    if (!qXmlFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
      throw IException(IException::Io,
                       QString("Unable to open xml file, [%1],  with write access").arg(xmlPath),
                       _FILEINFO_);
    }
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    settingsFromGroup3.save(writer, project);
    writer.writeEndDocument();
    qXmlFile.close();
    // read xml    
    qDebug() << "Testing XML: read XML to BundleObservationSolveSettings object...";
    XmlHandlerTester bsFromXml3(project, &reader, xmlFile);
    pvl = bsFromXml3.pvlObject("FromXml-3");
    cout << pvl << endl << endl;

    if (!qXmlFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
      throw IException(IException::Io,
                       QString("Unable to open xml file, [%1],  with write access").arg(xmlPath),
                       _FILEINFO_);
    }
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    settingsFromGroup4.save(writer, project);
    writer.writeEndDocument();
    qXmlFile.close();
    // read xml    
    qDebug() << "Testing XML: read XML to BundleObservationSolveSettings object...";
    XmlHandlerTester bsFromXml4(project, &reader, xmlFile);
    pvl = bsFromXml4.pvlObject("FromXml-4");
    cout << pvl << endl << endl;

    if (!qXmlFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
      throw IException(IException::Io,
                       QString("Unable to open xml file, [%1],  with write access").arg(xmlPath),
                       _FILEINFO_);
    }
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    boss.save(writer, project);
    writer.writeEndDocument();
    qXmlFile.close();
    // read xml    
    qDebug() << "Testing XML: read XML to BundleObservationSolveSettings object...";
    XmlHandlerTester bossToFill(project, &reader, xmlFile);
//    BundleObservationSolveSettings bossToFill(xmlFile, project, &reader);
    pvl = bossToFill.pvlObject("FromXml");
    cout << pvl << endl << endl;

    // read xml with no attributes or values
    qDebug() << "Testing XML: read XML with no attributes or values to object...";
    FileName emptyXmlFile("./unitTest_NoElementValues.xml");
    XmlHandlerTester bsFromEmptyXml(project, &reader, emptyXmlFile);
    pvl = bsFromEmptyXml.pvlObject("DefaultSettingsFromEmptyXml");
    cout << pvl << endl << endl;

    //bool deleted = qXmlFile.remove();
    //if (!deleted) {
    //  QString msg = "Unit Test failed. XML file [" + xmlPath + "not deleted.";
    //  throw IException(IException::Io, msg, _FILEINFO_);
    //}

    qDebug() << "Testing error throws...";
    try {
      BundleObservationSolveSettings::stringToInstrumentPointingSolveOption("Nonsense");
    } 
    catch (IException &e) {
      e.print();
    }
    try {
      BundleObservationSolveSettings::instrumentPointingSolveOptionToString(
          BundleObservationSolveSettings::InstrumentPointingSolveOption(-1));
    } 
    catch (IException &e) {
      e.print();
    }
    try {
      BundleObservationSolveSettings::stringToInstrumentPositionSolveOption("Nonsense");
    } 
    catch (IException &e) {
      e.print();
    }
    try {
      BundleObservationSolveSettings::instrumentPositionSolveOptionToString(
          BundleObservationSolveSettings::InstrumentPositionSolveOption(-2));
    } 
    catch (IException &e) {
      e.print();
    }
    settingsGroup1 += PvlKeyword("OVERHERMITE", "Nonsense");
    try {
      boss.setFromPvl(settingsGroup1);
    } 
    catch (IException &e) {
      e.print();
    }
    settingsGroup1 += PvlKeyword("OVEREXISTING", "Nonsense");
    try {
      boss.setFromPvl(settingsGroup1);
    } 
    catch (IException &e) {
      e.print();
    }
    settingsGroup1 += PvlKeyword("TWIST", "Nonsense");
    try {
      boss.setFromPvl(settingsGroup1);
    } 
    catch (IException &e) {
      e.print();
    }
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "Testing BundleImage...";
    #if 0
    TEST COVERAGE (SCOPE) FOR THIS SOURCE FILE: 100%
    #endif
    qDebug() << "Constructing BundleImage object from null camera...";
    Camera *camera = NULL;
    BundleImage bi(camera, "TestImageSerialNumber", "TestImageFileName");
    qDebug() << "setting null parentBundleObservation to BundleImage...";
    BundleObservationQsp parentObservation;
    bi.setParentObservation(parentObservation);
    qDebug() << "Access camera and parentObservation ...";
//    Camera *cam = bi.camera();
//    BundleObservation *parent = bi.parentObservation();
    qDebug() << "serial number = " << bi.serialNumber();
    qDebug() << "file name     = " << bi.fileName();
    qDebug() << "Testing copy constructor...";
    BundleImageQsp bi2 = BundleImageQsp( new BundleImage(bi) );
    qDebug() << "serial number = " << bi2->serialNumber();
    qDebug() << "file name     = " << bi2->fileName();
    qDebug() << "Testing assignment operator to set this equal to itself...";
    bi = bi;
    qDebug() << "serial number = " << bi.serialNumber();
    qDebug() << "file name     = " << bi.fileName();
    qDebug() << "Testing assignment operator to create a new object...";
    BundleImage bi3(camera, "TestImage2SerialNumber", "TestImage2FileName");
    bi3 = bi;
    qDebug() << "serial number = " << bi3.serialNumber();
    qDebug() << "file name     = " << bi3.fileName();
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "Testing BundleObservation...";
    qDebug() << "Constructing empty BundleObservation object...";
    #if 0
    TEST COVERAGE (SCOPE) FOR THIS SOURCE FILE: ??% (need to re-run SquishCoco)
    NEED:
        1) construct with image containing camera that is not null
          1a) construct also with non-null target body qsp
          1b) test initilaizeBodyRotation()
        2) NOT POSSIBLE - setSolveSettings - initParameterWeights() returns false
        3) initializeExteriorOrientation - positionOption!=No, pointingOption=No
        4) applyParameterCorrections -     positionOption!=No, pointingOption=No
          4a) Test the second exception thrown by applyParameterCorrections
          4b) Test successful call to applyParameterCorrections (#4249).
        5) initParameterWeights - possigma[0] == 0, possigma[1] == 0, possigma[2]  > 0
                                                    pntsigma[1]  > 0, pntsigma[2] == 0
        6) formatBundleOutputString - with instrumentPosition/instrumentRotation not NULL
    #endif
    BundleObservation bo;
    BundleTargetBodyQsp bundleTargetBody = BundleTargetBodyQsp(new BundleTargetBody);
    qDebug() << "Constructing BundleObservation object from BundleImage...";
    BundleObservation bo2(bi2, 
                          "ObservationNumber2", 
                          "InstrumentId2", 
                          bundleTargetBody);

    BundleImageQsp nullImage;
    BundleObservation nullBO(nullImage, 
                             "NullObservationNumber", 
                             "NullInstrumentId", 
                             bundleTargetBody);

    qDebug() << "Testing assignment operator to set this equal to itself...";
    bo2 = bo2;
    qDebug() << "Testing assignment operator to create a new object...";
    bo = bo2;
    qDebug() << "Testing copy constructor...";
    BundleObservation bo3(bo);

    qDebug() << "Testing mutators and accessors...";
    qDebug() << "    Set/get solve settings using with CAMESOLVE=NONE...";
    bo2.setSolveSettings(settingsFromGroup1);
    BundleObservationSolveSettings bossFromBo = *bo2.solveSettings();
    pvl = bossFromBo.pvlObject("NoCamAngles");
    cout << pvl << endl << endl;
    qDebug() << "    output bundle observation...";
    qDebug().noquote() << bo2.formatBundleOutputString(true);
    qDebug().noquote() << bo2.formatBundleOutputString(false);
    qDebug() << "    Set solve settings using with TWIST=FALSE...";
    bo2.setSolveSettings(bossToFill);
    bossFromBo = *bo2.solveSettings();
    pvl = bossFromBo.pvlObject("NoTwist");
    cout << pvl << endl << endl;
    qDebug() << "    output bundle observation...";
    qDebug().noquote() << bo2.formatBundleOutputString(true);
    qDebug().noquote() << bo2.formatBundleOutputString(false);
    qDebug() << "    Set solve settings using with CAMSOLVE=ALL and TWIST=TRUE...";
    bo3.setSolveSettings(bsFromEmptyXml);

    bossFromBo = *bo3.solveSettings();
    pvl = bossFromBo.pvlObject("SettingsFromBundleObservation");
    cout << pvl << endl << endl;
    bo3.setIndex(1);
    qDebug() << "index = " << toString(bo3.index());
    qDebug() << "instrument id = " << bo3.instrumentId();
    qDebug() << "number parameters =     " << toString(bo3.numberParameters());
    qDebug() << "number position param = " << toString(bo3.numberPositionParameters());
    qDebug() << "number pointing param = " << toString(bo3.numberPointingParameters());
    qDebug() << "parameter list: " << bo3.parameterList();
    qDebug() << "image names:    " << bo3.imageNames();

    LinearAlgebra::Vector paramWts = bo3.parameterWeights();
    LinearAlgebra::Vector paramCor = bo3.parameterCorrections();
    LinearAlgebra::Vector aprSigma = bo3.aprioriSigmas();
    LinearAlgebra::Vector adjSigma = bo3.adjustedSigmas();
    QString vectors = "parameter weights :     ";
    for (unsigned int i = 0; i < paramWts.size(); i++) {
      vectors.append(toString(paramWts[i]));
      vectors.append("     ");
    }
    vectors.append("\nparameter corrections : ");
    for (unsigned int i = 0; i < paramCor.size(); i++) {
      vectors.append(toString(paramCor[i]));
      vectors.append("     ");
    }
    vectors.append("\napriori sigmas :        ");
    for (unsigned int i = 0; i < aprSigma.size(); i++) {
      vectors.append(toString(aprSigma[i]));
      vectors.append("     ");
    }
    vectors.append("\nadjusted sigmas :       ");
    for (unsigned int i = 0; i < adjSigma.size(); i++) {
      vectors.append(toString(adjSigma[i]));
      vectors.append("     ");
    }
    qDebug().noquote() << vectors;
    
    // initializeBodyRotation (verify???)
    // bo3.initializeBodyRotation(); //Seg fault

    qDebug() << "    output bundle observation...";
    qDebug().noquote() << bo3.formatBundleOutputString(false);
    qDebug().noquote() << bo3.formatBundleOutputString(true);
    qDebug() << "init exterior orientiation successful?  " 
             << toString(bo3.initializeExteriorOrientation());
    //TODO: We should not have to catch an exception here, we need to use an observation
    //      with a better (i.e. non-null) Camera. See ticket #4249.
    try {
      qDebug() << "apply param corrections successful?     " 
               << toString(bo3.applyParameterCorrections(paramCor));
    }
    catch (IException &e) {
      e.print();
    }

    qDebug() << "";

    // spiceRotation and spicePosition (verify???)
    //SpiceRotation *rotation = 
    bo3.spiceRotation();
    //SpicePosition *position = 
    bo3.spicePosition();

    qDebug() << "    add another image...";
    bo3.append(
      BundleImageQsp(
        new BundleImage(camera, "TestImage2SerialNumber", "TestImage2FileName")));
    qDebug() << "    access images by serial number...";
    qDebug().noquote() << bo3.imageByCubeSerialNumber("TestImageSerialNumber")->fileName();
    qDebug().noquote() << bo3.imageByCubeSerialNumber("TestImage2SerialNumber")->fileName();
    qDebug() << "";

    //  See BundleObservation::applyParameterCorrections last catch (exception NOT thrown)
    qDebug() << "Testing exceptions...";
    BundleObservationSolveSettings bo3SettingsCopy(*(bo3.solveSettings()));
    try {
      bo3SettingsCopy.setInstrumentPositionSettings(BundleObservationSolveSettings::PositionOnly);
      nullBO.setSolveSettings(bo3SettingsCopy);
      nullBO.applyParameterCorrections(paramCor);
    }
    catch (IException &e) {
      e.print(); 
    }
    try {
      bo3.applyParameterCorrections(paramCor);
    }
    catch (IException &e) {
      e.print();
    }

    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "Testing BundleObservationVector...";
    qDebug() << "";
    #if 0
    TEST COVERAGE (SCOPE) FOR THIS SOURCE FILE: 21%
    Need:
      1) addNew - settings->solveObservationMode() == false
                  map.contains(obsNumber)
      2) addNew - settings->solveObservationMode()
                  map.contains(obsNumber) == false
      3) addNew - settings->solveObservationMode()
                  map.contains(obsNumber)
                  bo.instId() == this.instId
      4) addNew - settings->solveObservationMode()
                  map.contains(obsNumber)
                  bo.instId() != this.instId
                  bundleObservation != null
                  bundleSettings.numberSolveSettings() == 1
      5) addNew - settings->solveObservationMode()
                  map.contains(obsNumber)
                  bo.instId() != this.instId
                  bundleObservation != null
                  bundleSettings.numberSolveSettings() != 1
      6) addNew - 
                  map.contains(obsNumber)
                  bo.instId() != this.instId
                  bundleObservation == null
      7) initializeExteriorOrientation, numberPositionParameters, numberPointingParameters - size > 0
      8) getObsByCubeSerialNumber - map.contains(sn)
      9) getObsByCubeSerialNumber - map.contains(sn) == false

    #endif               
    BundleObservationVector bov;
    BundleSettingsQsp bundleSettings = BundleSettingsQsp(new BundleSettings);
    // BundleObservation *obs1 = bov.addnew(bi2, "obs1", "InstrumentIdBOV", bundleSettings);
    //qDebug() << obs1->formatBundleOutputString(true);
    //obs1 = bov.observationByCubeSerialNumber("obs1");
    //BundleObservation *obs2 = bov.addnew(bundleImage, "obs2", "InstrumentIdBOV", bundleSettings);
    //qDebug() << obs2->formatBundleOutputString(true);
    qDebug() << "number of position parameters: " << toString(bov.numberPositionParameters());
    qDebug() << "number of pointing parameters: " << toString(bov.numberPointingParameters());
    qDebug() << "number of parameters: " << toString(bov.numberParameters());
    
#if 0
    BundleObservation obs1b = *bov.getObservationByCubeSerialNumber("obs1");
    qDebug() << "same observation?" << toString((obs1 == obs1b));
    qDebug() << obs1b.formatBundleOutputString(true);
#endif
    // Following segfaults (see #4157)
    qDebug() << "init exterior orientiation successful?  " << toString(bov.initializeExteriorOrientation());
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "Testing BundleControlPoint...";
    #if 0
    TEST COVERAGE (SCOPE) FOR THIS SOURCE FILE: 100%
    #endif
    qDebug() << "Create FreePoint with free point containing 2 measures..."
        << "(note that first measure is ignored, second measure is not ignored)";
    ControlPoint *freePoint = new ControlPoint("FreePoint");
    ControlMeasure *cm1 = new ControlMeasure;
    cm1->SetCubeSerialNumber("Ignored");
    cm1->SetIgnored(true);
    freePoint->Add(cm1);
    ControlMeasure *cm2 = new ControlMeasure;
    cm2->SetCubeSerialNumber("NotIgnored");
    cm2->SetIgnored(false);
    cm2->SetCoordinate(1.0, 2.0);
    cm2->SetResidual(-3.0, 4.0);
    freePoint->Add(cm2);
    BundleControlPoint bcp1(freePoint);
    bool errorProp = false;
    double radiansToMeters = 10.0;

    qDebug() << "Type of BundleControlPoint 1:" << bcp1.type();

    bcp1.setRejected(true);
    qDebug() << "Set BundleControlPoint 1 to rejected - is rejected?"
        << toString(bcp1.isRejected());
    bcp1.setRejected(false);
    qDebug() << "Set BundleControlPoint 1 to non-rejected - is rejected?" 
        << toString(bcp1.isRejected());

    qDebug() << "Number of rejected measures:" << bcp1.numberOfRejectedMeasures();
    bcp1.setNumberOfRejectedMeasures(2);
    qDebug() << "Set number of rejected measures:" << bcp1.numberOfRejectedMeasures();
    bcp1.zeroNumberOfRejectedMeasures();
    qDebug() << "Zero out number of rejected measures:" << bcp1.numberOfRejectedMeasures();

    // ??? these print outs are not pretty... fix???
    qDebug().noquote() << bcp1.formatBundleOutputSummaryString(errorProp);
    // ??? these print outs are not pretty... fix???
    qDebug().noquote() << bcp1.formatBundleOutputDetailString(errorProp, radiansToMeters);
    errorProp = true;
    // ??? these print outs are not pretty... fix???
    qDebug().noquote() << bcp1.formatBundleOutputSummaryString(errorProp);
    // ??? these print outs are not pretty... fix???
    qDebug().noquote() << bcp1.formatBundleOutputDetailString(errorProp, radiansToMeters);
    qDebug() << "";

    qDebug() << "Modify FreePoint - setAdjustedSurfacePoint(0,0,10) and addMeasure()";
    SurfacePoint sp1(Latitude(0.0, Angle::Degrees), 
                     Longitude(0.0, Angle::Degrees), 
                     Distance(10.0, Distance::Meters));
    bcp1.setAdjustedSurfacePoint(sp1);
    // ??? this appears to do nothing! measure is added to the internal QVector of measures,
    // not the member control point...
    // probably need to fix the format string methods to use "this" instead of member control point
    // and accessor methods???
    BundleMeasure bcm = *(bcp1.addMeasure(cm1));
    // ??? these print outs are not pretty... fix???
    qDebug().noquote() << bcp1.formatBundleOutputSummaryString(errorProp);
    // ??? these print outs are not pretty... fix???
    qDebug().noquote() << bcp1.formatBundleOutputDetailString(errorProp, radiansToMeters);

    qDebug() << "Modify FreePoint - setWeights() - solveRadius=false";
    // default solveRadius=false
    BundleSettingsQsp settings = BundleSettingsQsp(new BundleSettings);
    double metersToRadians = 1.0 / radiansToMeters;
    bcp1.setWeights(settings, metersToRadians);
    qDebug().noquote() << bcp1.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp1.formatBundleOutputDetailString(errorProp, radiansToMeters);
    boost::numeric::ublas::bounded_vector< double, 3 > aprioriSigmas = bcp1.aprioriSigmas();
    boost::numeric::ublas::bounded_vector< double, 3 > weights = bcp1.weights();
    //??? never set 000??? init to 1.0e+50???
    boost::numeric::ublas::bounded_vector< double, 3 > corrections = bcp1.corrections();
    //??? never set 000??? 1.0e+50???
    boost::numeric::ublas::bounded_vector< double, 3 > adjustedSigmas = bcp1.adjustedSigmas();
    //??? never set 000c??? 1.0e+50???
    boost::numeric::ublas::bounded_vector< double, 3 > nicVector = bcp1.nicVector();
    SparseBlockRowMatrix qMatrix = bcp1.cholmodQMatrix(); //??? empty matrix...
    qDebug() << "aprioriSigmas:  "
               << (Isis::IsSpecial(aprioriSigmas[0]) ? "N/A" : Isis::toString(aprioriSigmas[0]))
               << (Isis::IsSpecial(aprioriSigmas[1]) ? "N/A" : Isis::toString(aprioriSigmas[1]))
               << (Isis::IsSpecial(aprioriSigmas[2]) ? "N/A" : Isis::toString(aprioriSigmas[2]));
    qDebug() << "weights:        " << weights[0] << weights[1] << weights[2];
    qDebug() << "corrections:    " << corrections[0] << corrections[1] << corrections[2];
    qDebug() << "adjustedSigmas: "
               << (Isis::IsSpecial(adjustedSigmas[0]) ? "N/A" : Isis::toString(adjustedSigmas[0]))
               << (Isis::IsSpecial(adjustedSigmas[1]) ? "N/A" : Isis::toString(adjustedSigmas[1]))
               << (Isis::IsSpecial(adjustedSigmas[2]) ? "N/A" : Isis::toString(adjustedSigmas[2]));
    qDebug() << "nicVector:      " << nicVector[0] << nicVector[1] << nicVector[2];
    qDebug() << "qMatrix:";
    qDebug() << qMatrix;

    qDebug() << "Residual rms:" << bcp1.residualRms();
    qDebug() << "";

    qDebug() << "Modify FreePoint - setWeights() - solveRadius=true, apriori lat/lon/rad <= 0";
    settings->setSolveOptions(false, false, false, true, Isis::Null);
    bcp1.setWeights(settings, metersToRadians);
    qDebug().noquote() << bcp1.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp1.formatBundleOutputDetailString(errorProp, radiansToMeters);
    aprioriSigmas = bcp1.aprioriSigmas();
    weights = bcp1.weights();
    qDebug() << "aprioriSigmas:  "
               << (Isis::IsSpecial(aprioriSigmas[0]) ? "N/A" : Isis::toString(aprioriSigmas[0]))
               << (Isis::IsSpecial(aprioriSigmas[1]) ? "N/A" : Isis::toString(aprioriSigmas[1]))
               << (Isis::IsSpecial(aprioriSigmas[2]) ? "N/A" : Isis::toString(aprioriSigmas[2]));
    qDebug() << "weights:        " << weights[0] << weights[1] << weights[2];
    qDebug() << "";

    qDebug() << "Modify FreePoint - setWeights() - solveRadius=true, apriori lat/lon/rad > 0";
    settings->setSolveOptions(false, false, false, true, 2.0, 3.0, 4.0);
    bcp1.setWeights(settings, metersToRadians);
    qDebug().noquote() << bcp1.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp1.formatBundleOutputDetailString(errorProp, radiansToMeters);
    aprioriSigmas = bcp1.aprioriSigmas();
    weights = bcp1.weights();
    qDebug() << "aprioriSigmas:  "
               << (Isis::IsSpecial(aprioriSigmas[0]) ? "N/A" : Isis::toString(aprioriSigmas[0]))
               << (Isis::IsSpecial(aprioriSigmas[1]) ? "N/A" : Isis::toString(aprioriSigmas[1]))
               << (Isis::IsSpecial(aprioriSigmas[2]) ? "N/A" : Isis::toString(aprioriSigmas[2]));
    qDebug() << "weights:        " << weights[0] << weights[1] << weights[2];
    qDebug() << "";

    ControlPoint *cp = bcp1.rawControlPoint();
    qDebug() << "Raw control point equal to original?    " << toString(*cp == *freePoint);
    qDebug() << "Raw control point is rejected?          " << toString(bcp1.isRejected());
    SurfacePoint sp = bcp1.adjustedSurfacePoint();
    qDebug() << "Adjusted SurfacePoint (Lat, Lon, Rad) = "
               << toString(sp.GetLatitude().degrees())
               << toString(sp.GetLongitude().degrees())
               << toString(sp.GetLocalRadius().meters());
    qDebug() << "";

    qDebug() << "Create FixedPoint from empty fixed point, adjusted surface point (90, 180, 10)...";
    ControlPoint *fixedPoint = new ControlPoint("FixedPoint");
    fixedPoint->SetType(ControlPoint::Fixed);
    BundleControlPoint *bcp3 = new BundleControlPoint(fixedPoint);
    SurfacePoint sp2(Latitude(90.0, Angle::Degrees), 
                     Longitude(180.0, Angle::Degrees), 
                     Distance(10.0, Distance::Meters));
    bcp3->setAdjustedSurfacePoint(sp2);
    qDebug().noquote() << bcp3->formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp3->formatBundleOutputDetailString(errorProp, radiansToMeters);

    qDebug() << "Modify FixedPoint - setWeights()";
    bcp3->setWeights(settings, metersToRadians);
    qDebug().noquote() << bcp3->formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp3->formatBundleOutputDetailString(errorProp, radiansToMeters);
    aprioriSigmas = bcp3->aprioriSigmas();
    weights = bcp3->weights();
    qDebug() << "aprioriSigmas:  "
               << (Isis::IsSpecial(aprioriSigmas[0]) ? "NULL" : Isis::toString(aprioriSigmas[0]))
               << (Isis::IsSpecial(aprioriSigmas[1]) ? "NULL" : Isis::toString(aprioriSigmas[1]))
               << (Isis::IsSpecial(aprioriSigmas[2]) ? "NULL" : Isis::toString(aprioriSigmas[2]));
    qDebug() << "weights:        " << weights[0] << weights[1] << weights[2];
    qDebug() << "";

    qDebug() << "Create ConstrainedPoint from empty constrained point, surface point (0, 0, 10)...";
    ControlPoint *constrainedPoint = new ControlPoint("ConstrainedPoint");
    constrainedPoint->SetType(ControlPoint::Constrained);
    BundleControlPoint bcp4(constrainedPoint);
    bcp4.setAdjustedSurfacePoint(sp1);
    qDebug().noquote() << bcp4.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp4.formatBundleOutputDetailString(errorProp, radiansToMeters);

    qDebug() << "Modify ConstrainedPoint - setWeights() - solveRadius=false";
    settings->setSolveOptions(false, false, false, false);
    bcp4.setWeights(settings, metersToRadians);
    qDebug().noquote() << bcp4.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp4.formatBundleOutputDetailString(errorProp, radiansToMeters);
    aprioriSigmas = bcp4.aprioriSigmas();
    weights = bcp4.weights();
    qDebug() << "aprioriSigmas:  "
               << (Isis::IsSpecial(aprioriSigmas[0]) ? "NULL" : Isis::toString(aprioriSigmas[0]))
               << (Isis::IsSpecial(aprioriSigmas[1]) ? "NULL" : Isis::toString(aprioriSigmas[1]))
               << (Isis::IsSpecial(aprioriSigmas[2]) ? "NULL" : Isis::toString(aprioriSigmas[2]));
    qDebug() << "weights:        " << weights[0] << weights[1] << weights[2];
    qDebug() << "";

    qDebug() << "Modify ConstrainedPoint - setWeights() - no constraints, solveRadius=true, "
                "apriori lat/lon/rad <= 0";
    settings->setSolveOptions(false, false, false, true);
    bcp4.setWeights(settings, metersToRadians);
    qDebug().noquote() << bcp4.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp4.formatBundleOutputDetailString(errorProp, radiansToMeters);
    aprioriSigmas = bcp4.aprioriSigmas();
    weights = bcp4.weights();
    qDebug() << "aprioriSigmas:  "
               << (Isis::IsSpecial(aprioriSigmas[0]) ? "NULL" : Isis::toString(aprioriSigmas[0]))
               << (Isis::IsSpecial(aprioriSigmas[1]) ? "NULL" : Isis::toString(aprioriSigmas[1]))
               << (Isis::IsSpecial(aprioriSigmas[2]) ? "NULL" : Isis::toString(aprioriSigmas[2]));
    qDebug() << "weights:        " << weights[0] << weights[1] << weights[2];
    qDebug() << "";

    qDebug() << "Modify ConstrainedPoint - setWeights() - no constraints, solveRadius=true, "
                "apriori lat/lon/rad > 0";
    settings->setSolveOptions(false, false, false, true, 2.0, 3.0, 4.0);
    bcp4.setWeights(settings, metersToRadians);
    qDebug().noquote() << bcp4.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp4.formatBundleOutputDetailString(errorProp, radiansToMeters);
    aprioriSigmas = bcp4.aprioriSigmas();
    weights = bcp4.weights();
    qDebug() << "aprioriSigmas:  "
               << (Isis::IsSpecial(aprioriSigmas[0]) ? "NULL" : Isis::toString(aprioriSigmas[0]))
               << (Isis::IsSpecial(aprioriSigmas[1]) ? "NULL" : Isis::toString(aprioriSigmas[1]))
               << (Isis::IsSpecial(aprioriSigmas[2]) ? "NULL" : Isis::toString(aprioriSigmas[2]));
    qDebug() << "weights:        " << weights[0] << weights[1] << weights[2];
    qDebug() << "";
    qDebug() << "";

    qDebug() << "Create ConstrainedPoint from constrained point with adjusted surface point "
                "(32, 120, 1000)...";
    SurfacePoint aprioriSurfPt;
    aprioriSurfPt.SetRadii(Distance(1000.0, Distance::Meters),
                           Distance(1000.0, Distance::Meters),
                           Distance(1000.0, Distance::Meters));
    boost::numeric::ublas::symmetric_matrix<double, boost::numeric::ublas::upper> covar;
    covar.resize(3);
    covar.clear();
    covar(0,0) = 100.0;
    covar(1,1) = 2500.0;
    covar(2,2) = 400.0;
    aprioriSurfPt.SetRectangular(Displacement(-424.024048, Displacement::Meters),
                                 Displacement(734.4311949, Displacement::Meters),
                                 Displacement(529.919264, Displacement::Meters), covar);
    constrainedPoint->SetAprioriSurfacePoint(aprioriSurfPt);
    BundleControlPoint bcp5(constrainedPoint);
    SurfacePoint adjustedSurfPt(constrainedPoint->GetAdjustedSurfacePoint());
    adjustedSurfPt.SetSpherical(Latitude(32., Angle::Degrees),
                                Longitude(120., Angle::Degrees),
                                Distance(1000., Distance::Meters),
                                Angle(1.64192315,Angle::Degrees),
                                Angle(1.78752107, Angle::Degrees),
                                Distance(38.454887335682053718134171237789, Distance::Meters));
    adjustedSurfPt.SetRadii(Distance(1000.0, Distance::Meters),
                            Distance(1000.0, Distance::Meters),
                            Distance(1000.0, Distance::Meters));
    bcp5.setAdjustedSurfacePoint(adjustedSurfPt);
    qDebug().noquote() << bcp5.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp5.formatBundleOutputDetailString(errorProp, radiansToMeters);
    qDebug() << "Modify ConstrainedPoint - setWeights() - solveRadius=t, lat/lon/rad constrained";
    bcp5.setWeights(settings, metersToRadians);
    qDebug().noquote() << bcp5.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp5.formatBundleOutputDetailString(errorProp, radiansToMeters);
    aprioriSigmas = bcp5.aprioriSigmas(); // these values were verified by comparing against
                                          // SurfacePoint truth data
    weights = bcp5.weights();
    qDebug() << "aprioriSigmas:  "
               << (Isis::IsSpecial(aprioriSigmas[0]) ? "NULL" : Isis::toString(aprioriSigmas[0]))
               << (Isis::IsSpecial(aprioriSigmas[1]) ? "NULL" : Isis::toString(aprioriSigmas[1]))
               << (Isis::IsSpecial(aprioriSigmas[2]) ? "NULL" : Isis::toString(aprioriSigmas[2]));
    qDebug() << "weights:        " << weights[0] << weights[1] << weights[2];
    qDebug() << "";

    qDebug() << "Create copy of FreePoint using copy constructor...";
    BundleControlPoint bcp2(bcp1);
    qDebug().noquote() << bcp2.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp2.formatBundleOutputDetailString(errorProp, radiansToMeters);
    qDebug() << "";

    qDebug() << "Overwrite existing object with FixedPoint information...";
    bcp2.copy(*bcp3);
    qDebug().noquote() << bcp2.formatBundleOutputSummaryString(errorProp);
    qDebug().noquote() << bcp2.formatBundleOutputDetailString(errorProp, radiansToMeters);
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "Testing BundleMeasure...";

    // TEST COVERAGE (SCOPE) FOR THIS SOURCE FILE: 86% //TODO update when SquishCoco works again
    BundleMeasure bundleMeasure(cm2, bcp3);

    try {
      bundleMeasure.observationSolveSettings();
    }
    catch (IException &e) {
      e.print();
    }
    bundleMeasure.setParentObservation(BundleObservationQsp(new BundleObservation(bo2)));
    // const BundleObservationSolveSettings *solveSettings = 
    bundleMeasure.observationSolveSettings();
    // Camera *cam = 
    bundleMeasure.camera();
    // BundleObservation  *parentObs = 
    bundleMeasure.parentBundleObservation();
    BundleControlPoint *parentBCP = bundleMeasure.parentControlPoint();
    qDebug() << "parent control point id" << parentBCP->id();
    bundleMeasure.setParentImage(BundleImageQsp(new BundleImage(bi)));
    BundleImageQsp parentImage = bundleMeasure.parentBundleImage();
    qDebug() << "parent image id" << parentImage->serialNumber();

    // Copy and =
    BundleMeasure bundleMeasureRejected(bundleMeasure); // We will use this to test setRejected.
    BundleMeasure bundleMeasureEq = bundleMeasure;

    // Test setRejected(true)
    bundleMeasureRejected.setRejected(true);

    // Test self-assignment
    bundleMeasure = bundleMeasure;

    qDebug() << "";
    // Verify state and copies
    printBundleMeasure(bundleMeasure);
    printBundleMeasure(bundleMeasureRejected);
    printBundleMeasure(bundleMeasureEq);
      
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "";

    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug() << "Testing BundleTargetBody...";
    qDebug() << "";

    qDebug() << "Create an empty BundleTargetBody";
    qDebug() << "";
    BundleTargetBody btb1;

    qDebug().noquote() << btb1.formatBundleOutputString(false);

    //TODO test creating a BundleTargetBody from a target

    qDebug() << "";
    qDebug() << "Convert strings to TargetRadiiSolveMethods and back";
    qDebug() << "";

    BundleTargetBody::TargetRadiiSolveMethod targetRadiiSolveMethod;
    targetRadiiSolveMethod = BundleTargetBody::stringToTargetRadiiOption("none");
    qDebug() << targetRadiiSolveMethod;
    qDebug() << BundleTargetBody::targetRadiiOptionToString(targetRadiiSolveMethod);
    targetRadiiSolveMethod = BundleTargetBody::stringToTargetRadiiOption("mean");
    qDebug() << targetRadiiSolveMethod;
    qDebug() << BundleTargetBody::targetRadiiOptionToString(targetRadiiSolveMethod);
    targetRadiiSolveMethod = BundleTargetBody::stringToTargetRadiiOption("all");
    qDebug() << targetRadiiSolveMethod;
    qDebug() << BundleTargetBody::targetRadiiOptionToString(targetRadiiSolveMethod);

    qDebug() << "";
    qDebug() << "Setup the BundleTargetBody to solve for everything but mean radius";
    qDebug() << "";
    // We do not test solving for acceleration as it is not implemented.
    std::set<int> targetParameterSolveCodes;
    targetParameterSolveCodes.insert(BundleTargetBody::PoleRA);
    targetParameterSolveCodes.insert(BundleTargetBody::VelocityPoleRA);
//     targetParameterSolveCodes.insert(BundleTargetBody::AccelerationPoleRA);
    targetParameterSolveCodes.insert(BundleTargetBody::PoleDec);
    targetParameterSolveCodes.insert(BundleTargetBody::VelocityPoleDec);
//     targetParameterSolveCodes.insert(BundleTargetBody::AccelerationPoleDec);
    targetParameterSolveCodes.insert(BundleTargetBody::PM);
    targetParameterSolveCodes.insert(BundleTargetBody::VelocityPM);
//     targetParameterSolveCodes.insert(BundleTargetBody::AccelerationPM);
    targetParameterSolveCodes.insert(BundleTargetBody::TriaxialRadiusA);
    targetParameterSolveCodes.insert(BundleTargetBody::TriaxialRadiusB);
    targetParameterSolveCodes.insert(BundleTargetBody::TriaxialRadiusC);
    double poleRA               = -2.0;
    double poleRASigma          = -0.2;
    double poleRAVelocity       = -3.0;
    double poleRAVelocitySigma  = -3.5;
    double poleDec              = -4.0;
    double poleDecSigma         = -5.0;
    double poleDecVelocity      = -6.0;
    double poleDecVelocitySigma = -7.0;
    double pm                   = -8.0;
    double pmSigma              = -9.0;
    double pmVelocity           = -10.0;
    double pmVelocitySigma      = -11.0;
    double aRadius              = 12.0;
    double aRadiusSigma         = 13.0;
    double bRadius              = 14.0;
    double bRadiusSigma         = 0.001;
    double cRadius              = 15.0;
    double cRadiusSigma         = 17.0;
    double meanRadius           = 20.0;
    double meanRadiusSigma      = 21.0;
    btb1.setSolveSettings(
        targetParameterSolveCodes,
        Angle(poleRA, Angle::Degrees),             Angle(poleRASigma, Angle::Degrees),
        Angle(poleRAVelocity, Angle::Degrees),     Angle(poleRAVelocitySigma, Angle::Degrees),
        Angle(poleDec, Angle::Degrees),            Angle(poleDecSigma, Angle::Degrees),
        Angle(poleDecVelocity, Angle::Degrees),    Angle(poleDecVelocitySigma, Angle::Degrees),
        Angle(pm, Angle::Degrees),                 Angle(pmSigma, Angle::Degrees),
        Angle(pmVelocity, Angle::Degrees),         Angle(pmVelocitySigma, Angle::Degrees),
        (BundleTargetBody::TargetRadiiSolveMethod)2,
        Distance(aRadius, Distance::Kilometers),   Distance(aRadiusSigma, Distance::Kilometers),
        Distance(bRadius, Distance::Kilometers),   Distance(bRadiusSigma, Distance::Kilometers),
        Distance(cRadius, Distance::Kilometers),   Distance(cRadiusSigma, Distance::Kilometers),
        Distance(meanRadius ,Distance::Kilometers),Distance(meanRadiusSigma, Distance::Kilometers));
    qDebug().noquote() << btb1.formatBundleOutputString(false);
    qDebug() << "";
    qDebug().noquote() << btb1.formatBundleOutputString(true);
    LinearAlgebra::Vector btb1Weights = btb1.parameterWeights();
    QString btb1WString;
    for (size_t i = 0; i < btb1Weights.size(); i++) {
      btb1WString.append(toString(btb1Weights[i]));
      if (i < btb1Weights.size() - 1) {
        btb1WString.append(", ");
      }
    }
    qDebug().noquote() << btb1WString;

    qDebug() << "";
    qDebug() << "Apply some corrections";
    qDebug() << "";

    LinearAlgebra::Vector btb1CumCorrections (btb1.numberParameters());
    btb1CumCorrections = btb1.parameterCorrections();
    QString btb1CString;
    for (size_t i = 0; i < btb1CumCorrections.size(); i++) {
      btb1CString.append(toString(btb1CumCorrections[i]));
      if (i < btb1CumCorrections.size() - 1) {
        btb1CString.append(", ");
      }
    }
    qDebug().noquote() << btb1CString;
    qDebug() << "";
    LinearAlgebra::Vector btb1Corrections (btb1.numberParameters());
    for (size_t i = 0; i < btb1Corrections.size(); i++) {
      btb1Corrections[i] = pow(-0.7, i);
    }
    btb1.applyParameterCorrections(btb1Corrections);
    qDebug().noquote() << btb1.formatBundleOutputString(true);
    btb1Weights = btb1.parameterWeights();
    btb1WString.clear();
    for (size_t i = 0; i < btb1Weights.size(); i++) {
      btb1WString.append(toString(btb1Weights[i]));
      if (i < btb1Weights.size() - 1) {
        btb1WString.append(", ");
      }
    }
    qDebug().noquote() << btb1WString;
    qDebug() << "";
    btb1CumCorrections = btb1.parameterCorrections();
    btb1CString.clear();
    for (size_t i = 0; i < btb1CumCorrections.size(); i++) {
      btb1CString.append(toString(btb1CumCorrections[i]));
      if (i < btb1CumCorrections.size() - 1) {
        btb1CString.append(", ");
      }
    }
    qDebug().noquote() << btb1CString;
    qDebug() << "";
    for (size_t i = 0; i < btb1Corrections.size(); i++) {
      btb1Corrections[i] = pow(1.1, i);
    }
    btb1.applyParameterCorrections(btb1Corrections);
    qDebug().noquote() << btb1.formatBundleOutputString(true);
    btb1Weights = btb1.parameterWeights();
    btb1WString.clear();
    for (size_t i = 0; i < btb1Weights.size(); i++) {
      btb1WString.append(toString(btb1Weights[i]));
      if (i < btb1Weights.size() - 1) {
        btb1WString.append(", ");
      }
    }
    qDebug().noquote() << btb1WString;
    qDebug() << "";
    btb1CumCorrections = btb1.parameterCorrections();
    btb1CString.clear();
    for (size_t i = 0; i < btb1CumCorrections.size(); i++) {
      btb1CString.append(toString(btb1CumCorrections[i]));
      if (i < btb1CumCorrections.size() - 1) {
        btb1CString.append(", ");
      }
    }
    qDebug().noquote() << btb1CString;

    qDebug() << "";
    qDebug() << "Test acccessors";
    qDebug() << "";
    qDebug() << "Pole Right Ascension";
    qDebug() << btb1.solvePoleRA() << btb1.solvePoleRAVelocity() << btb1.solvePoleRAAcceleration();
    qDebug() << "Pole Declination";
    qDebug() << btb1.solvePoleDec() << btb1.solvePoleDecVelocity() << btb1.solvePoleDecAcceleration();
    qDebug() << "Prime Meridian";
    qDebug() << btb1.solvePM() << btb1.solvePMVelocity() << btb1.solvePMAcceleration();
    qDebug() << "Radii";
    qDebug() << btb1.solveTriaxialRadii() << btb1.solveMeanRadius();
    qDebug() << "Parameter Count";
    qDebug() << btb1.numberRadiusParameters() << btb1.numberParameters();
    qDebug() << "Parameter Solutions";
    LinearAlgebra::Vector btb1Solutions = btb1.parameterSolution();
    QString btb1SString;
    for (size_t i = 0; i < btb1Solutions.size(); i++) {
      btb1SString.append(toString(btb1Solutions[i]));
      if (i < btb1Solutions.size() - 1) {
        btb1SString.append(", ");
      }
    }
    qDebug().noquote() << btb1SString;
    qDebug() << "Apriori Sigmas";
    LinearAlgebra::Vector btb1Apriori = btb1.aprioriSigmas();
    QString btb1AprioriString;
    for (size_t i = 0; i < btb1Apriori.size(); i++) {
      btb1AprioriString.append(toString(btb1Apriori[i]));
      if (i < btb1Apriori.size() - 1) {
        btb1AprioriString.append(", ");
      }
    }
    qDebug().noquote() << btb1AprioriString;
    qDebug() << "Adjusted Sigmas";
    LinearAlgebra::Vector btb1Adjusted = btb1.adjustedSigmas();
    QString btb1AdjustedString;
    for (size_t i = 0; i < btb1Adjusted.size(); i++) {
      btb1AdjustedString.append(toString(btb1Adjusted[i]));
      if (i < btb1Adjusted.size() - 1) {
        btb1AdjustedString.append(", ");
      }
    }
    qDebug().noquote() << btb1AdjustedString;
    qDebug() << "Pole Right Ascension Coefficients";
    std::vector<Angle> btb1RACoefs = btb1.poleRaCoefs();
    QString btb1RACoefString;
    for (size_t i = 0; i < btb1RACoefs.size(); i++) {
      btb1RACoefString.append(btb1RACoefs[i].toString());
      if (i < btb1RACoefs.size() - 1) {
        btb1RACoefString.append(", ");
      }
    }
    qDebug().noquote() << btb1RACoefString;
    qDebug() << "Pole Declination Coefficients";
    std::vector<Angle> btb1DecCoefs = btb1.poleDecCoefs();
    QString btb1DecCoefString;
    for (size_t i = 0; i < btb1DecCoefs.size(); i++) {
      btb1DecCoefString.append(btb1DecCoefs[i].toString());
      if (i < btb1DecCoefs.size() - 1) {
        btb1DecCoefString.append(", ");
      }
    }
    qDebug().noquote() << btb1DecCoefString;
    qDebug() << "Prime Meridian Coefficients";
    std::vector<Angle> btb1PMCoefs = btb1.pmCoefs();
    QString btb1PMCoefString;
    for (size_t i = 0; i < btb1PMCoefs.size(); i++) {
      btb1PMCoefString.append(btb1PMCoefs[i].toString());
      if (i < btb1PMCoefs.size() - 1) {
        btb1PMCoefString.append(", ");
      }
    }
    qDebug().noquote() << btb1PMCoefString;
    qDebug() << "VtPV";
    qDebug() << btb1.vtpv();
    qDebug() << "Local Radius";
    qDebug().noquote()
      << btb1.localRadius(Latitude(15, Angle::Degrees), Longitude(15, Angle::Degrees)).toString();

    qDebug() << "";
    qDebug() << "Test copy constructor";
    qDebug() << "";
    BundleTargetBody btb3(btb1);
    qDebug().noquote() << btb3.formatBundleOutputString(true);

    qDebug() << "Switch free and valid sigmas";
    qDebug() << "";
    poleRASigma          = 0.2;
    poleRAVelocitySigma  = 3.5;
    poleDecSigma         = 5.0;
    poleDecVelocitySigma = 7.0;
    pmSigma              = 9.0;
    pmVelocitySigma      = 11.0;
    aRadiusSigma         = 0.0;
    bRadiusSigma         = 0.0;
    cRadiusSigma         = 0.0;
    btb1.setSolveSettings(
        targetParameterSolveCodes,
        Angle(poleRA, Angle::Degrees),             Angle(poleRASigma, Angle::Degrees),
        Angle(poleRAVelocity, Angle::Degrees),     Angle(poleRAVelocitySigma, Angle::Degrees),
        Angle(poleDec, Angle::Degrees),            Angle(poleDecSigma, Angle::Degrees),
        Angle(poleDecVelocity, Angle::Degrees),    Angle(poleDecVelocitySigma, Angle::Degrees),
        Angle(pm, Angle::Degrees),                 Angle(pmSigma, Angle::Degrees),
        Angle(pmVelocity, Angle::Degrees),         Angle(pmVelocitySigma, Angle::Degrees),
        (BundleTargetBody::TargetRadiiSolveMethod)2,
        Distance(aRadius, Distance::Kilometers),   Distance(aRadiusSigma, Distance::Kilometers),
        Distance(bRadius, Distance::Kilometers),   Distance(bRadiusSigma, Distance::Kilometers),
        Distance(cRadius, Distance::Kilometers),   Distance(cRadiusSigma, Distance::Kilometers),
        Distance(meanRadius ,Distance::Kilometers),Distance(meanRadiusSigma, Distance::Kilometers));
    qDebug().noquote() << btb1.formatBundleOutputString(true);
    btb1Weights = btb1.parameterWeights();
    btb1WString.clear();
    for (size_t i = 0; i < btb1Weights.size(); i++) {
      btb1WString.append(toString(btb1Weights[i]));
      if (i < btb1Weights.size() - 1) {
        btb1WString.append(", ");
      }
    }
    qDebug().noquote() << btb1WString;

    qDebug() << "";
    qDebug() << "Test assignment operator";
    qDebug() << "";
    qDebug() << "Self assignment";
    btb3 = btb3;
    qDebug().noquote() << btb3.formatBundleOutputString(true);
    qDebug() << "Assignment to other";
    btb3 = btb1;
    qDebug().noquote() << btb3.formatBundleOutputString(true);

    qDebug() << "Setup a BundleTargetBody that solves for only mean radius";
    qDebug() << "";
    BundleTargetBody btb2;
    targetParameterSolveCodes.clear();
    targetParameterSolveCodes.insert(BundleTargetBody::MeanRadius);
    btb2.setSolveSettings(
        targetParameterSolveCodes,
        Angle(poleRA, Angle::Degrees),             Angle(poleRASigma, Angle::Degrees),
        Angle(poleRAVelocity, Angle::Degrees),     Angle(poleRAVelocitySigma, Angle::Degrees),
        Angle(poleDec, Angle::Degrees),            Angle(poleDecSigma, Angle::Degrees),
        Angle(poleDecVelocity, Angle::Degrees),    Angle(poleDecVelocitySigma, Angle::Degrees),
        Angle(pm, Angle::Degrees),                 Angle(pmSigma, Angle::Degrees),
        Angle(pmVelocity, Angle::Degrees),         Angle(pmVelocitySigma, Angle::Degrees),
        (BundleTargetBody::TargetRadiiSolveMethod)1,
        Distance(aRadius, Distance::Kilometers),   Distance(aRadiusSigma, Distance::Kilometers),
        Distance(bRadius, Distance::Kilometers),   Distance(bRadiusSigma, Distance::Kilometers),
        Distance(cRadius, Distance::Kilometers),   Distance(cRadiusSigma, Distance::Kilometers),
        Distance(meanRadius ,Distance::Kilometers),Distance(meanRadiusSigma, Distance::Kilometers));
    qDebug().noquote() << btb2.formatBundleOutputString(true);
    LinearAlgebra::Vector btb2Weights = btb2.parameterWeights();
    QString btb2WString;
    for (size_t i = 0; i < btb2Weights.size(); i++) {
      btb2WString.append(toString(btb2Weights[i]));
      if (i < btb2Weights.size() - 1) {
        btb2WString.append(", ");
      }
    }
    qDebug().noquote() << btb2WString;
    qDebug() << "";
    qDebug().noquote() << btb2.meanRadius().toString();
    qDebug() << "";
    qDebug() << "Switch free and valid sigmas";
    qDebug() << "";
    meanRadiusSigma = 0;
    btb2.setSolveSettings(
        targetParameterSolveCodes,
        Angle(poleRA, Angle::Degrees),             Angle(poleRASigma, Angle::Degrees),
        Angle(poleRAVelocity, Angle::Degrees),     Angle(poleRAVelocitySigma, Angle::Degrees),
        Angle(poleDec, Angle::Degrees),            Angle(poleDecSigma, Angle::Degrees),
        Angle(poleDecVelocity, Angle::Degrees),    Angle(poleDecVelocitySigma, Angle::Degrees),
        Angle(pm, Angle::Degrees),                 Angle(pmSigma, Angle::Degrees),
        Angle(pmVelocity, Angle::Degrees),         Angle(pmVelocitySigma, Angle::Degrees),
        (BundleTargetBody::TargetRadiiSolveMethod)1,
        Distance(aRadius, Distance::Kilometers),   Distance(aRadiusSigma, Distance::Kilometers),
        Distance(bRadius, Distance::Kilometers),   Distance(bRadiusSigma, Distance::Kilometers),
        Distance(cRadius, Distance::Kilometers),   Distance(cRadiusSigma, Distance::Kilometers),
        Distance(meanRadius ,Distance::Kilometers),Distance(meanRadiusSigma, Distance::Kilometers));
    qDebug().noquote() << btb2.formatBundleOutputString(true);
    btb2Weights = btb2.parameterWeights();
    btb2WString.clear();
    for (size_t i = 0; i < btb2Weights.size(); i++) {
      btb2WString.append(toString(btb2Weights[i]));
      if (i < btb2Weights.size() - 1) {
        btb2WString.append(", ");
      }
    }
    qDebug().noquote() << btb2WString;

    qDebug() << "";
    qDebug() << "Test reading from a PvlObject";
    qDebug() << "";
    PvlGroup goodRAGroup;
    goodRAGroup += PvlKeyword("Ra", "velocity");
    goodRAGroup += PvlKeyword("RaValue", "15");
    goodRAGroup += PvlKeyword("RaSigma", "0.487");
    goodRAGroup += PvlKeyword("RaVelocityValue", "10");
    goodRAGroup += PvlKeyword("RaVelocitySigma", "1.01");
    PvlGroup goodDecGroup;
    goodDecGroup += PvlKeyword("Dec", "velocity");
    goodDecGroup += PvlKeyword("DecValue", "25");
    goodDecGroup += PvlKeyword("DecSigma", "2.3");
    goodDecGroup += PvlKeyword("DecVelocityValue", "5");
    goodDecGroup += PvlKeyword("DecVelocitySigma", "0.03");
    PvlGroup goodPMGroup;
    goodPMGroup += PvlKeyword("PM", "velocity");
    goodPMGroup += PvlKeyword("PmValue", "20");
    goodPMGroup += PvlKeyword("PmSigma", "2.4");
    goodPMGroup += PvlKeyword("PmVelocityValue", "30");
    goodPMGroup += PvlKeyword("pmVelocitySigma", "10");
    PvlGroup goodRadiiGroup;
    goodRadiiGroup += PvlKeyword("RadiiSolveOption", "triaxial");
    goodRadiiGroup += PvlKeyword("RadiusAValue", "2");
    goodRadiiGroup += PvlKeyword("RadiusASigma", "0.2");
    goodRadiiGroup += PvlKeyword("RadiusBValue", "3");
    goodRadiiGroup += PvlKeyword("RadiusBSigma", "0.3");
    goodRadiiGroup += PvlKeyword("RadiuscValue", "4");
    goodRadiiGroup += PvlKeyword("RadiuscSigma", "0.4");
    PvlObject goodBTBObject;
    goodBTBObject += goodRAGroup;
    goodBTBObject += goodDecGroup;
    goodBTBObject += goodPMGroup;
    goodBTBObject += goodRadiiGroup;
    btb3.readFromPvl(goodBTBObject);
    qDebug().noquote() << btb3.formatBundleOutputString(true);

    qDebug() << "Test error throws";
    qDebug() << "";

    // Correction errors (parameters and corrections mismatch)
    try {
      btb1.applyParameterCorrections(LinearAlgebra::Vector(btb1.numberParameters() + 1));
    }
    catch (IException &e) {
      e.print();
    }

    // Internal correction errors - the corrections vector contains a non-Null special Isis
    // pixel value (e.g. Hrs, Lrs...), and one of our parameter solve codes is for an angle.
    // This causes the Angle(double, Angle::Radians) to throw an exception.
    try {
      LinearAlgebra::Vector hasSpecial(btb1.numberParameters());
      hasSpecial[0] = Isis::Lrs; 
      btb1.applyParameterCorrections(hasSpecial);
    }
    catch (IException &e) {
      e.print();
    }

    // Radii solve method errors
    try {
      BundleTargetBody::stringToTargetRadiiOption("Invalid Method");
    }
    catch (IException &e) {
      e.print();
    }
    try {
      BundleTargetBody::targetRadiiOptionToString((BundleTargetBody::TargetRadiiSolveMethod)-1);
    }
    catch (IException &e) {
      e.print();
    }

    // Radii accessor errors
    try {
      btb2.radii();
    }
    catch (IException &e) {
      e.print();
    }
    try {
      btb1.meanRadius();
    }
    catch (IException &e) {
      e.print();
    }

    // local radius error
    try {
      btb2.localRadius(Latitude(15, Angle::Degrees), Longitude(15, Angle::Degrees));
    }
    catch (IException &e) {
      e.print();
    }

    // Read Pvl errors
    PvlObject badBTBObject;
    PvlGroup badRaPosGroup;
    badRaPosGroup += PvlKeyword("RaValue", "Not a double");
    badBTBObject += badRaPosGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRaPosSigGroup;
    badRaPosSigGroup += PvlKeyword("RaSigma", "Also not a double");
    badBTBObject += badRaPosSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRaVelGroup;
    badRaVelGroup += PvlKeyword("RaVelocityValue", "Still not a double");
    badBTBObject += badRaVelGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRaVelSigGroup;
    badRaVelSigGroup += PvlKeyword("RaVelocitySigma", "Definitely not a double");
    badBTBObject += badRaVelSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRaAccGroup;
    badRaAccGroup += PvlKeyword("RaAccelerationValue", "A string");
    badBTBObject += badRaAccGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRaAccSigGroup;
    badRaAccSigGroup += PvlKeyword("RaAccelerationSigma", "Also a string");
    badBTBObject += badRaAccSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badDecPosGroup;
    badDecPosGroup += PvlKeyword("DecValue", "Another string");
    badBTBObject += badDecPosGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badDecPosSigGroup;
    badDecPosSigGroup += PvlKeyword("DecSigma", "The seventh string");
    badBTBObject += badDecPosSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badDecVelGroup;
    badDecVelGroup += PvlKeyword("DecVelocityValue", "The loneliest string");
    badBTBObject += badDecVelGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badDecVelSigGroup;
    badDecVelSigGroup += PvlKeyword("DecVelocitySigma", "The happy string");
    badBTBObject += badDecVelSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badDecAccGroup;
    badDecAccGroup += PvlKeyword("DecAccelerationValue", "The fast string");
    badBTBObject += badDecAccGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badDecAccSigGroup;
    badDecAccSigGroup += PvlKeyword("DecAccelerationSigma", "The wobbling string");
    badBTBObject += badDecAccSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badPMGroup;
    badPMGroup += PvlKeyword("PmValue", "Or are they char arrays?");
    badBTBObject += badPMGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badPMSigGroup;
    badPMGroup += PvlKeyword("PmSigma", "This one is");
    badBTBObject += badPMGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badPMVelGroup;
    badPMVelGroup += PvlKeyword("PmVelocityValue", "This is also a char array");
    badBTBObject += badPMVelGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badPMVelSigGroup;
    badPMVelSigGroup += PvlKeyword("pmVelocitySigma", "These still aren't doubles");
    badBTBObject += badPMVelSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badPMAccGroup;
    badPMAccGroup += PvlKeyword("PmAccelerationValue", "And that's what matters");
    badBTBObject += badPMAccGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badPMAccSigGroup;
    badPMAccSigGroup += PvlKeyword("PmAccelerationSigma", "The eighteenth not double");
    badBTBObject += badPMAccSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRadAGroup;
    badRadAGroup += PvlKeyword("RadiusAValue", "The twentieth not double");
    badBTBObject += badRadAGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup negRadAGroup;
    negRadAGroup += PvlKeyword("RadiusAValue", "-8");
    badBTBObject += negRadAGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRadASigGroup;
    badRadASigGroup += PvlKeyword("RadiusASigma", "The true twentieth not double");
    badBTBObject += badRadASigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup negRadASigGroup;
    negRadASigGroup += PvlKeyword("RadiusASigma", "-7");
    badBTBObject += negRadASigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRadBGroup;
    badRadBGroup += PvlKeyword("RadiusBValue", "Only five more");
    badBTBObject += badRadBGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup negRadBGroup;
    negRadBGroup += PvlKeyword("RadiusBValue", "-6");
    badBTBObject += negRadBGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRadBSigGroup;
    badRadBSigGroup += PvlKeyword("RadiusBSigma", "Only four more");
    badBTBObject += badRadBSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup negRadBSigGroup;
    negRadBSigGroup += PvlKeyword("RadiusBSigma", "-5");
    badBTBObject += negRadBSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRadCGroup;
    badRadCGroup += PvlKeyword("RadiusCValue", "Only three more");
    badBTBObject += badRadCGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup negRadCGroup;
    negRadCGroup += PvlKeyword("RadiusCValue", "-4");
    badBTBObject += negRadCGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badRadCSigGroup;
    badRadCSigGroup += PvlKeyword("RadiusCSigma", "Only two more");
    badBTBObject += badRadCSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup negRadCSigGroup;
    negRadCSigGroup += PvlKeyword("RadiusCSigma", "-3");
    badBTBObject += negRadCSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badMeanRadGroup;
    badMeanRadGroup += PvlKeyword("MeanRadiusValue", "Only one more");
    badBTBObject += badMeanRadGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup negMeanRadGroup;
    negMeanRadGroup += PvlKeyword("MeanRadiusValue", "-2");
    badBTBObject += negMeanRadGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup badMeanRadSigGroup;
    badMeanRadSigGroup += PvlKeyword("MeanRadiusSigma", "The end");
    badBTBObject += badMeanRadSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    badBTBObject.clear();
    PvlGroup negMeanRadSigGroup;
    negMeanRadSigGroup += PvlKeyword("MeanRadiusSigma", "-1");
    badBTBObject += negMeanRadSigGroup;
    try {
      btb3.readFromPvl(badBTBObject);
    }
    catch (IException &e) {
      e.print();
    }
    qDebug() << "";
    qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
  } 
  catch (IException &e) {
    e.print();
  }

}


/**
 * Outputs the BundleMeasure state fo testing and verification
 *
 * @param m The BundleMeasure to print information on
 */
 void printBundleMeasure(BundleMeasure &m) {
   qDebug() << "rejected?" << toString(m.isRejected());
   qDebug() << "measure sample " << toString(m.sample());
   qDebug() << "measure line   " << toString(m.line());
   qDebug() << "sample residual" << toString(m.sampleResidual());
   qDebug() << "line residual" << toString(m.lineResidual());
   qDebug() << "residual magnitude" << toString(m.residualMagnitude());
   qDebug() << "measure serial number" << m.cubeSerialNumber();
   qDebug() << "focal x" << toString(m.focalPlaneMeasuredX());
   qDebug() << "focal y" << toString(m.focalPlaneMeasuredY());
   qDebug() << "computed focal x" << toString(m.focalPlaneComputedX());
   qDebug() << "computed focal y" << toString(m.focalPlaneComputedY());
   qDebug() << "observation index" << toString(m.observationIndex());
   qDebug() << "";  
}
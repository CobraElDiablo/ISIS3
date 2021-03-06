#ifndef hiLab_h
#define hiLab_h
/**
 * @file
 * $Revision: 1.3 $
 * $Date: 2008/05/14 21:07:26 $
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for
 *   intellectual property information,user agreements, and related information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software
 *   and related material nor shall the fact of distribution constitute any such
 *   warranty, and no responsibility is assumed by the USGS in connection
 *   therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see
 *   the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */

#include "Cube.h"
#include "Pvl.h"

namespace Isis {
  /**
   * @brief Process HiRise label
   *
   * This class retrieves label keyword values from an Isis3
   * HiRise cube file. This class receives a Cube object from
   * an opened HiRise cube file and has methods to return HiRise
   * specific keyword values from the label.
   *
   * @ingroup MarsReconnaissanceOrbiter
   *
   * @author 2005-06-29 unknown
   *
   * @internal
   *  @history 2005-06-29 unknown - Original Version
   *  @history 2006-08-17 Debbie A. Cook - Added members p_bin and p_tdi
   *                          along with methods to retrieve them and the ccd
   *  @history 2008-05-12 Steven Lambright - Removed references to CubeInfo
   *  @history 2012-04-16 Jeannie Walldren - Wrapped class inside Isis
   *                          namespace. Added documentation.
   */
  class HiLab {
    public:
      HiLab(Cube *cube);
  
      /**
       * Returns the value of the CpmmNumber keyword read from the instrument 
       * group in the labels of a hiris cube 
       *  
       * @return int The cpmm number from the cube's labels
       */
      int getCpmmNumber() {
        return p_cpmmNumber;
      };
  
      /**
       * Returns the value of the ChannelNumber keyword read from the instrument
       * group in the labels of a hirise cube 
       *  
       * @return int The channel number from the cube's labels
       */
      int getChannel() {
        return p_channel;
      };
  
      /**
       * Returns the bin value, read as the value of the Summing keyword from 
       * the instrument group in the labels of a hirise cube 
       *  
       * @return int The bin value from the cube's labels
       */        
      int getBin() {
        return p_bin;
      };

      /**
       * Returns the value of the Tdi keyword read from the instrument group in 
       * the labels of a hirise cube 
       *  
       * @return int The tdi from the cube's labels
       */  
      int getTdi() {
        return p_tdi;
      };
  
      /**
       * Returns the value of the ccd from a lookup table based on 
       * the cpmm number. 
       *  
       *  
       * @return int The ccd value.
       */
      int getCcd();
  
    private:
      int p_cpmmNumber;  /**< Value of the CpmmNumber keyword from the cube's 
                               Instrument group**/
      int p_channel;     /**< Value of the ChannelNumber keyword from the 
                               cube's Instrument group**/
      int p_bin;         /**< Value of the Summing keyword from the cube's 
                               Instrument group, if it exists**/
      int p_tdi;         /**< Value of the Tdi keyword from the cube's 
                               Instrument group, if it exists**/
  };
};
#endif

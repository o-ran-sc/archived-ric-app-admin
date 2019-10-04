/*
==================================================================================
        Copyright (c) 2018-2019 AT&T Intellectual Property.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
==================================================================================
*/
/*
 * ric_indication.h
 *
 *  Created on: Aug 14, 2019
 *      Author:  Ashwin Sridharan
 */

#ifndef SGNB_INDICATION_HELPER_
#define SGNB_INDICATION_HELPER_

  
#include <iostream>
#include <vector>

struct erab_sgnb_item {
  erab_sgnb_item(void):  qci(0), priority_level(0), pre_emption_capability(0), pre_emption_vulnerability(0),  dl_forwarding(0), ul_configuration(0), secondary_meNB_UTL_GTP(0), transport_layer_addr(0), transport_layer_addr_size(0), transport_layer_addr_unused(0), gtp_tei(0), gtp_tei_size(0), rlc_mode(0) {};

  /* Erab level qos parameters */
  long int qci;
  long int priority_level;
  long int pre_emption_capability;
  long int pre_emption_vulnerability;

 
  
  /* Optional  forwarding info */
  void * dl_forwarding;
  void * ul_configuration;
  void * secondary_meNB_UTL_GTP;
  
  /* Tunnel end point */
  unsigned char * transport_layer_addr;
  int transport_layer_addr_size;
  int transport_layer_addr_unused;
  
  unsigned char * gtp_tei;
  int gtp_tei_size;

  /* Rlc mode */
  long int rlc_mode;
};



struct erab_item {

  erab_item(void):erab_id(0), drb_id(0), cause(0), pdcp_at_sgnb(0), mcg_resources(0), scg_resources(0), sgnb_pdcp_present(0) {};
  
  long int erab_id;
  long int drb_id;

  int cause; // used when not admitting 
  /* enums describing type
     of resources present */

  long int pdcp_at_sgnb;
  long int mcg_resources;
  long int scg_resources;


  int sgnb_pdcp_present;
  struct erab_sgnb_item sgnb_item;

};


  

class sgnb_addition_helper {

  
public:

  sgnb_addition_helper(void): menb_ue_x2ap_id(0), sgnb_ue_x2ap_id(0), encryption_algs(0), encryption_algs_size(0), encryption_algs_unused(0), integrity_protection_algs(0), integrity_protection_algs_size(0), integrity_protection_algs_unused(0), sgnb_security_key(0), sgnb_security_key_size(0), sgnb_security_key_unused(0), bit_rate_max_up(0), bit_rate_max_dn(0), menb_sgnb_container(0), menb_sgnb_container_size(0), selected_plmn(0), selected_plmn_size(0), plmn_identity(0), plmn_identity_size(0), eutran_identifier(0), eutran_identifier_size(0), eutran_identifier_unused(0), subscriber_profile_id(0), cause(0), cause_desc(0) {};
  
  long int menb_ue_x2ap_id, sgnb_ue_x2ap_id;
  
  unsigned char *encryption_algs;
  int encryption_algs_size;
  int encryption_algs_unused;
  
  unsigned char *integrity_protection_algs;
  int integrity_protection_algs_size;
  int integrity_protection_algs_unused;
  
  unsigned char *sgnb_security_key;
  int sgnb_security_key_size;
  int sgnb_security_key_unused;
  
  long int  bit_rate_max_up, bit_rate_max_dn;
  
  unsigned char *menb_sgnb_container;
  int menb_sgnb_container_size;

  unsigned char *selected_plmn;
  int selected_plmn_size;
  
  unsigned char *plmn_identity;
  int plmn_identity_size;

  unsigned char *eutran_identifier;
  int eutran_identifier_size;
  int eutran_identifier_unused;


  int subscriber_profile_id;
  
  int cause; //  CHOICE {radioNetwork CauseRadioNetwork,transport CauseTransport,protocol CauseProtocol,misc CauseMisc}
  int cause_desc; //enum CauseRadioNetwork, enum CauseTransport...etc
  
  void clear(void){
    erab_list.clear();
    erab_admitted_list.clear();
    erab_not_admitted_list.clear();
  };
  
  std::vector<struct erab_item> * get_list(void) { return &erab_list;};
  std::vector<struct erab_item> * get_admitted_list(void) { return &erab_admitted_list;};
  std::vector<struct erab_item> * get_not_admitted_list(void) { return &erab_not_admitted_list;};
  
  std::vector<struct erab_item> erab_list;
  std::vector<struct erab_item> erab_admitted_list;
  std::vector<struct erab_item> erab_not_admitted_list;


};

#endif

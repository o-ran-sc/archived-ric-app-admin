/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-PDU-Contents"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_X2N_E_RABs_Admitted_ToBeReleased_SgNBModAckList_H_
#define	_X2N_E_RABs_Admitted_ToBeReleased_SgNBModAckList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct X2N_ProtocolIE_Single_Container;

/* X2N_E-RABs-Admitted-ToBeReleased-SgNBModAckList */
typedef struct X2N_E_RABs_Admitted_ToBeReleased_SgNBModAckList {
	A_SEQUENCE_OF(struct X2N_ProtocolIE_Single_Container) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} X2N_E_RABs_Admitted_ToBeReleased_SgNBModAckList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_X2N_E_RABs_Admitted_ToBeReleased_SgNBModAckList;

#ifdef __cplusplus
}
#endif

#endif	/* _X2N_E_RABs_Admitted_ToBeReleased_SgNBModAckList_H_ */
#include <asn_internal.h>

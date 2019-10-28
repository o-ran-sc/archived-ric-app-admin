/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-PDU-Contents"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_X2N_ServedNRcellsENDCX2ManagementList_H_
#define	_X2N_ServedNRcellsENDCX2ManagementList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include "X2N_ServedNRCell-Information.h"
#include <constr_SEQUENCE.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct X2N_NRNeighbour_Information;
struct X2N_ProtocolExtensionContainer;

/* Forward definitions */
typedef struct X2N_ServedNRcellsENDCX2ManagementList__Member {
	X2N_ServedNRCell_Information_t	 servedNRCellInfo;
	struct X2N_NRNeighbour_Information	*nRNeighbourInfo;	/* OPTIONAL */
	struct X2N_ProtocolExtensionContainer	*iE_Extensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ServedNRcellsENDCX2ManagementList__Member;

/* X2N_ServedNRcellsENDCX2ManagementList */
typedef struct X2N_ServedNRcellsENDCX2ManagementList {
	A_SEQUENCE_OF(ServedNRcellsENDCX2ManagementList__Member) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} X2N_ServedNRcellsENDCX2ManagementList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_X2N_ServedNRcellsENDCX2ManagementList;

#ifdef __cplusplus
}
#endif

#endif	/* _X2N_ServedNRcellsENDCX2ManagementList_H_ */
#include <asn_internal.h>

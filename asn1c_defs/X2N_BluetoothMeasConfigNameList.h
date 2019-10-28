/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-IEs"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_X2N_BluetoothMeasConfigNameList_H_
#define	_X2N_BluetoothMeasConfigNameList_H_


#include <asn_application.h>

/* Including external dependencies */
#include "X2N_BluetoothName.h"
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* X2N_BluetoothMeasConfigNameList */
typedef struct X2N_BluetoothMeasConfigNameList {
	A_SEQUENCE_OF(X2N_BluetoothName_t) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} X2N_BluetoothMeasConfigNameList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_X2N_BluetoothMeasConfigNameList;
extern asn_SET_OF_specifics_t asn_SPC_X2N_BluetoothMeasConfigNameList_specs_1;
extern asn_TYPE_member_t asn_MBR_X2N_BluetoothMeasConfigNameList_1[1];
extern asn_per_constraints_t asn_PER_type_X2N_BluetoothMeasConfigNameList_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _X2N_BluetoothMeasConfigNameList_H_ */
#include <asn_internal.h>

/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-IEs"
 * 	found in "../../asn_defs/asn1/x2ap-modified-15-05.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_ERABActivityNotifyItemList_H_
#define	_ERABActivityNotifyItemList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct ERABActivityNotifyItem;

/* ERABActivityNotifyItemList */
typedef struct ERABActivityNotifyItemList {
	A_SEQUENCE_OF(struct ERABActivityNotifyItem) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ERABActivityNotifyItemList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ERABActivityNotifyItemList;

#ifdef __cplusplus
}
#endif

#endif	/* _ERABActivityNotifyItemList_H_ */
#include <asn_internal.h>
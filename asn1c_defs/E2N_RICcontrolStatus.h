/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "E2AP-IEs"
 * 	found in "../../asn_defs/asn1/e2ap-v031-subset.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_E2N_RICcontrolStatus_H_
#define	_E2N_RICcontrolStatus_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum E2N_RICcontrolStatus {
	E2N_RICcontrolStatus_success	= 0,
	E2N_RICcontrolStatus_rejected	= 1,
	E2N_RICcontrolStatus_failed	= 2
	/*
	 * Enumeration is extensible
	 */
} e_E2N_RICcontrolStatus;

/* E2N_RICcontrolStatus */
typedef long	 E2N_RICcontrolStatus_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_E2N_RICcontrolStatus;
asn_struct_free_f E2N_RICcontrolStatus_free;
asn_struct_print_f E2N_RICcontrolStatus_print;
asn_constr_check_f E2N_RICcontrolStatus_constraint;
ber_type_decoder_f E2N_RICcontrolStatus_decode_ber;
der_type_encoder_f E2N_RICcontrolStatus_encode_der;
xer_type_decoder_f E2N_RICcontrolStatus_decode_xer;
xer_type_encoder_f E2N_RICcontrolStatus_encode_xer;
per_type_decoder_f E2N_RICcontrolStatus_decode_uper;
per_type_encoder_f E2N_RICcontrolStatus_encode_uper;
per_type_decoder_f E2N_RICcontrolStatus_decode_aper;
per_type_encoder_f E2N_RICcontrolStatus_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _E2N_RICcontrolStatus_H_ */
#include <asn_internal.h>

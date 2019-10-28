/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-IEs"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_X2N_RRC_Context_H_
#define	_X2N_RRC_Context_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* X2N_RRC-Context */
typedef OCTET_STRING_t	 X2N_RRC_Context_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_X2N_RRC_Context;
asn_struct_free_f X2N_RRC_Context_free;
asn_struct_print_f X2N_RRC_Context_print;
asn_constr_check_f X2N_RRC_Context_constraint;
ber_type_decoder_f X2N_RRC_Context_decode_ber;
der_type_encoder_f X2N_RRC_Context_encode_der;
xer_type_decoder_f X2N_RRC_Context_decode_xer;
xer_type_encoder_f X2N_RRC_Context_encode_xer;
per_type_decoder_f X2N_RRC_Context_decode_uper;
per_type_encoder_f X2N_RRC_Context_encode_uper;
per_type_decoder_f X2N_RRC_Context_decode_aper;
per_type_encoder_f X2N_RRC_Context_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _X2N_RRC_Context_H_ */
#include <asn_internal.h>

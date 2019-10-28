/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-IEs"
 * 	found in "../../asn_defs/asn1/X2AP-minimized.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_E2N_CauseRadioNetwork_H_
#define	_E2N_CauseRadioNetwork_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum E2N_CauseRadioNetwork {
	E2N_CauseRadioNetwork_handover_desirable_for_radio_reasons	= 0,
	E2N_CauseRadioNetwork_time_critical_handover	= 1,
	E2N_CauseRadioNetwork_resource_optimisation_handover	= 2,
	E2N_CauseRadioNetwork_reduce_load_in_serving_cell	= 3,
	E2N_CauseRadioNetwork_partial_handover	= 4,
	E2N_CauseRadioNetwork_unknown_new_eNB_UE_X2AP_ID	= 5,
	E2N_CauseRadioNetwork_unknown_old_eNB_UE_X2AP_ID	= 6,
	E2N_CauseRadioNetwork_unknown_pair_of_UE_X2AP_ID	= 7,
	E2N_CauseRadioNetwork_ho_target_not_allowed	= 8,
	E2N_CauseRadioNetwork_tx2relocoverall_expiry	= 9,
	E2N_CauseRadioNetwork_trelocprep_expiry	= 10,
	E2N_CauseRadioNetwork_cell_not_available	= 11,
	E2N_CauseRadioNetwork_no_radio_resources_available_in_target_cell	= 12,
	E2N_CauseRadioNetwork_invalid_MME_GroupID	= 13,
	E2N_CauseRadioNetwork_unknown_MME_Code	= 14,
	E2N_CauseRadioNetwork_encryption_and_or_integrity_protection_algorithms_not_supported	= 15,
	E2N_CauseRadioNetwork_reportCharacteristicsEmpty	= 16,
	E2N_CauseRadioNetwork_noReportPeriodicity	= 17,
	E2N_CauseRadioNetwork_existingMeasurementID	= 18,
	E2N_CauseRadioNetwork_unknown_eNB_Measurement_ID	= 19,
	E2N_CauseRadioNetwork_measurement_temporarily_not_available	= 20,
	E2N_CauseRadioNetwork_unspecified	= 21,
	/*
	 * Enumeration is extensible
	 */
	E2N_CauseRadioNetwork_load_balancing	= 22,
	E2N_CauseRadioNetwork_handover_optimisation	= 23,
	E2N_CauseRadioNetwork_value_out_of_allowed_range	= 24,
	E2N_CauseRadioNetwork_multiple_E_RAB_ID_instances	= 25,
	E2N_CauseRadioNetwork_switch_off_ongoing	= 26,
	E2N_CauseRadioNetwork_not_supported_QCI_value	= 27,
	E2N_CauseRadioNetwork_measurement_not_supported_for_the_object	= 28,
	E2N_CauseRadioNetwork_tDCoverall_expiry	= 29,
	E2N_CauseRadioNetwork_tDCprep_expiry	= 30,
	E2N_CauseRadioNetwork_action_desirable_for_radio_reasons	= 31,
	E2N_CauseRadioNetwork_reduce_load	= 32,
	E2N_CauseRadioNetwork_resource_optimisation	= 33,
	E2N_CauseRadioNetwork_time_critical_action	= 34,
	E2N_CauseRadioNetwork_target_not_allowed	= 35,
	E2N_CauseRadioNetwork_no_radio_resources_available	= 36,
	E2N_CauseRadioNetwork_invalid_QoS_combination	= 37,
	E2N_CauseRadioNetwork_encryption_algorithms_not_aupported	= 38,
	E2N_CauseRadioNetwork_procedure_cancelled	= 39,
	E2N_CauseRadioNetwork_rRM_purpose	= 40,
	E2N_CauseRadioNetwork_improve_user_bit_rate	= 41,
	E2N_CauseRadioNetwork_user_inactivity	= 42,
	E2N_CauseRadioNetwork_radio_connection_with_UE_lost	= 43,
	E2N_CauseRadioNetwork_failure_in_the_radio_interface_procedure	= 44,
	E2N_CauseRadioNetwork_bearer_option_not_supported	= 45,
	E2N_CauseRadioNetwork_mCG_Mobility	= 46,
	E2N_CauseRadioNetwork_sCG_Mobility	= 47,
	E2N_CauseRadioNetwork_count_reaches_max_value	= 48,
	E2N_CauseRadioNetwork_unknown_old_en_gNB_UE_X2AP_ID	= 49,
	E2N_CauseRadioNetwork_pDCP_Overload	= 50
} e_E2N_CauseRadioNetwork;

/* E2N_CauseRadioNetwork */
typedef long	 E2N_CauseRadioNetwork_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_E2N_CauseRadioNetwork_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_E2N_CauseRadioNetwork;
extern const asn_INTEGER_specifics_t asn_SPC_CauseRadioNetwork_specs_1;
asn_struct_free_f CauseRadioNetwork_free;
asn_struct_print_f CauseRadioNetwork_print;
asn_constr_check_f CauseRadioNetwork_constraint;
ber_type_decoder_f CauseRadioNetwork_decode_ber;
der_type_encoder_f CauseRadioNetwork_encode_der;
xer_type_decoder_f CauseRadioNetwork_decode_xer;
xer_type_encoder_f CauseRadioNetwork_encode_xer;
per_type_decoder_f CauseRadioNetwork_decode_uper;
per_type_encoder_f CauseRadioNetwork_encode_uper;
per_type_decoder_f CauseRadioNetwork_decode_aper;
per_type_encoder_f CauseRadioNetwork_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _E2N_CauseRadioNetwork_H_ */
#include <asn_internal.h>

/*
 * Generated by asn1c-0.9.29 n1 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-PDU-Contents"
 * 	found in "../../asn_defs/asn1/x2ap-15-04.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -gen-PER -no-gen-OER`
 */

#ifndef	_X2N_UE_ContextInformation_H_
#define	_X2N_UE_ContextInformation_H_


#include <asn_application.h>

/* Including external dependencies */
#include "X2N_UE-S1AP-ID.h"
#include "X2N_UESecurityCapabilities.h"
#include "X2N_AS-SecurityInformation.h"
#include "X2N_UEAggregateMaximumBitRate.h"
#include "X2N_SubscriberProfileIDforRFP.h"
#include "X2N_E-RABs-ToBeSetup-List.h"
#include "X2N_RRC-Context.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct X2N_HandoverRestrictionList;
struct X2N_LocationReportingInformation;
struct X2N_ProtocolExtensionContainer;

/* X2N_UE-ContextInformation */
typedef struct X2N_UE_ContextInformation {
	X2N_UE_S1AP_ID_t	 mME_UE_S1AP_ID;
	X2N_UESecurityCapabilities_t	 uESecurityCapabilities;
	X2N_AS_SecurityInformation_t	 aS_SecurityInformation;
	X2N_UEAggregateMaximumBitRate_t	 uEaggregateMaximumBitRate;
	X2N_SubscriberProfileIDforRFP_t	*subscriberProfileIDforRFP;	/* OPTIONAL */
	X2N_E_RABs_ToBeSetup_List_t	 e_RABs_ToBeSetup_List;
	X2N_RRC_Context_t	 rRC_Context;
	struct X2N_HandoverRestrictionList	*handoverRestrictionList;	/* OPTIONAL */
	struct X2N_LocationReportingInformation	*locationReportingInformation;	/* OPTIONAL */
	struct X2N_ProtocolExtensionContainer	*iE_Extensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} X2N_UE_ContextInformation_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_X2N_UE_ContextInformation;

#ifdef __cplusplus
}
#endif

#endif	/* _X2N_UE_ContextInformation_H_ */
#include <asn_internal.h>
#include "eventtype.h"

static EErrorReason EER[] = {
    {ERRC_SUCCESS, "OK."},
    {ERRC_INVALID_MSG, "Invalid message."},
    {ERRC_INVALID_DATA, "Invalid data."},
    {ERRC_METHOD_NOT_ALLOWED, "Method not allowed."},
    {ERRC_PROCESS_FAILED, "Process failed."},
    {ERRC_BIKE_IS_TAKEN, "Bike is taken."},
    {ERRC_BIKE_IS_RUNNING, "Bike is running."},
    {ERRC_BIKE_IS_DAMAGED, "Bike is damaged."},
    {ERR_NULL, "Undefined."}}; /* added for for iteration walkthrough */

const char *getReasonByErrorCode(i32 code) {
  i32 i = 0;
  for (i = 0; EER[i].code != ERR_NULL; i++) {
    if (EER[i].code == code) return EER[i].reason;
  }
  return EER[i].reason;  // "Undefined"
}
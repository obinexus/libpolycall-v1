#ifndef POLYCALL_FFI_H
#define POLYCALL_FFI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// FFI Banking API
typedef struct ffi_credit_score {
    uint32_t score;
    uint32_t card_level;
    uint8_t unlocked;
    double credit_limit;
} ffi_credit_score_t;

typedef struct ffi_transaction {
    char from_bank[64];
    char to_bank[64];
    double amount;
    uint32_t status;
} ffi_transaction_t;

typedef struct ffi_contract {
    uint16_t source_port;
    uint16_t target_port;
    uint8_t is_open;
    uint64_t metrics_count;
} ffi_contract_t;

// Core FFI functions
int32_t ffi_banking_init(void);
void ffi_banking_cleanup(void);
int32_t ffi_banking_transaction(const char* from, const char* to, 
                                double amount, ffi_transaction_t* result);
int32_t ffi_contract_open(uint16_t src_port, uint16_t dst_port, 
                          ffi_contract_t* contract);
int32_t ffi_contract_close(ffi_contract_t* contract);
int32_t ffi_get_credit_score(const char* user_id, uint32_t base_score,
                             ffi_credit_score_t* score);
uint8_t ffi_debit_validate(const char* account_id, double amount,
                           double current_balance);
int32_t ffi_micro_isolate(const char* service_name);

#ifdef __cplusplus
}
#endif

#endif // POLYCALL_FFI_H

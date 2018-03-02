/*
 * token_verifier.h
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_TOKEN_VERIFIER_H_
#define INC_TASKS_TOKEN_VERIFIER_H_

enum token_verifier_status_table {
    TOKEN_VERIFIER_RUNNING  = 0,
    TOKEN_VERIFIER_STOPPING = 1,
    TOKEN_VERIFIER_STOPPED  = 2
};

extern void token_verifier_verify_token(char *token);
extern void token_verifier_task(void *pvParameter);

#endif /* INC_TASKS_TOKEN_VERIFIER_H_ */

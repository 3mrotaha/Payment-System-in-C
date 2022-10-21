#include <stdio.h>
#include <string.h>
#include "../global.h"
#include "server.h"
#include "../Database/database.h"
#include "../Card/card.h"

// use this variable to store the current client account data (pan and balance)
static ST_accountDB_t accountData;

/*
This function is used to take all transaction data and validate its data.
it check the account details and amount availability.
and also update the data base with the new balance.
*/
EN_transState_t recieveTransactionData(ST_transaction_t *transData)
{
    f32_t tempBalance;
    if (isValidAccount(transData->cardHolderData) == OK_SERVER)
    {
        tempBalance = accountData.balance;
        // check if the balance in data base is bigger than the transaction amount
        if (isAmountAvailable(transData->terminalData) == OK_SERVER)
        {
            // replace the old balance with the new one
            accountData.balance -= transData->terminalData->transAmount; // update the new balance
            writeData(&accountData);                     // save the new balance into data base
        }
        else{
            transData->terminalData->transAmount = 0; // clear the amount to not be subtracted from the existing balance        
            return DECLINED_INSUFFECIENT_FUND; // if the amount isn't available        
        }
        
    }
    else 
        return DECLINED_STOLENCARD;       // if the PAN is wrong

    // this condition is to make sure the balance updated 
    if (accountData.balance == tempBalance)
        return INTERNAL_SERVER_ERROR; // in case an error accured
    else
    {
        saveTransaction(transData->transactionSequenceNumber); // save the transaction sequence Number
        return APPROVED;        // in case no error accured
    }
}




// This function is to check if the PAN is valid or not
EN_serverError_t isValidAccount(ST_cardData_t *cardData)
{
    strcpy(accountData.primaryAccountNumber, cardData->primaryAccountNumber);
    if (searchData(&accountData) == OK_DATABASE)
        return OK_SERVER;

    return DECLINED_STOLENCARD;
}




// This function is to check if the transaction amount is less than the balance or not
EN_serverError_t isAmountAvailable(ST_terminalData_t *termData)
{
    readData(&accountData);
    if (termData->transAmount < accountData.balance)
    {
        return OK_SERVER;
    }

    return LOW_BALANCE;
}




// This function is used to save transaction for other uses.
EN_serverError_t saveTransaction(ST_transaction_t *transData)
{
    static uint32_t sequenceNum = 1;
    transData->transactionSequenceNumber = sequenceNum++;
    saveLog(*transData); // save the transaction data into log file
    return OK_SERVER;
}




// This function is used to get transaction for other uses.
EN_serverError_t getTransaction(uint32_t transactionSequenceNumber, ST_transaction_t *transData)
{
    transData->transactionSequenceNumber = transactionSequenceNumber;
    if (getLog(transData) == OK_DATABASE)
        return OK_SERVER;

    return TRANSACTION_NOT_FOUND;
}

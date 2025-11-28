#include "lrun.h"
#include "SharedParameter.h"
#include "globals.h"
#include "web_api.h"
#include <stdio.h>
#include <string.h>

Inventory()
{
    char customerId[64];
    char itemsList[2048];
    char body[8192];
    char temp[512];
    char itemCode[64];
    char uom[32];
    char bearerHeader[2048];
    char *itemToken;

    // Read parameters
    strcpy(customerId, lr_eval_string("{cusId}"));
    strcpy(itemsList, lr_eval_string("{items}"));

    sprintf(bearerHeader, "Bearer %s", lr_eval_string("{authToken}"));
    lr_output_message("Authorization header: %.50s...", bearerHeader);

    // Set API headers
    web_add_auto_header("Authorization", bearerHeader);
    web_add_auto_header("Content-Type", "application/json");
    web_add_auto_header("Accept", "application/json");

    // Build request body
    strcpy(body, "{\n  \"productRequests\": [\n");
    itemToken = strtok(itemsList, ",");

    while (itemToken != NULL) {
        while (*itemToken == ' ') itemToken++;

        memset(itemCode, 0, sizeof(itemCode));
        memset(uom, 0, sizeof(uom));
        sscanf(itemToken, "%[^-]-%s", itemCode, uom);

        sprintf(temp,
            "    {\n"
            "      \"productInfo\": {\n"
            "        \"itemCode\": \"%s\",\n"
            "        \"quantity\": 1,\n"
            "        \"unitOfMeasure\": \"%s\"\n"
            "      }\n"
            "    }",
            itemCode, uom);

        strcat(body, temp);
        itemToken = strtok(NULL, ",");
        if (itemToken) strcat(body, ",\n");
        else strcat(body, "\n");
    }

    strcat(body, "  ]\n}");
    lr_save_string(body, "reqBody");

    lr_output_message("Generated Body:\n%s", body);

    web_reg_save_param_ex("ParamName=responseBody", "LB=", "RB=", "NotFound=Warning", LAST);

    web_custom_request("Customer_Inventory",
        "URL=https://api.edp.henryschein.com/api/customers/{cusId}/inventory",
        "Method=POST",
        "Resource=0",
        "RecContentType=application/json",
        "Mode=HTTP",
        "Body={reqBody}",
        LAST);

    lr_output_message("API Response:\n%s", lr_eval_string("{responseBody}"));

    return 0;
}

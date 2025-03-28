#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#define BUFFER_SIZE 1024

// Function to execute a command and print its output.
void run_command(const char *cmd) {
    FILE *fp;
    char buffer[BUFFER_SIZE];

    fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Error: Could not run command: %s\n", cmd);
        return;
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
    pclose(fp);
}

// Function to URL-decode a string (basic version)
void url_decode(char *src, char *dest) {
    char *pstr = src;
    char *pbuf = dest;
    while (*pstr) {
        if (*pstr == '%') {
            if (pstr[1] && pstr[2]) {
                char hex[3] = { pstr[1], pstr[2], '\0' };
                *pbuf++ = (char) strtol(hex, NULL, 16);
                pstr += 3;
            }
        } else if (*pstr == '+') {
            *pbuf++ = ' ';
            pstr++;
        } else {
            *pbuf++ = *pstr++;
        }
    }
    *pbuf = '\0';
}

// Simple function to get the value of a query string parameter
// Returns 1 if found, 0 otherwise. The result is copied into `value`.
int get_query_param(const char *query, const char *param, char *value, size_t max_len) {
    char *pos = strstr(query, param);
    if (!pos)
        return 0;

    pos += strlen(param);
    if (*pos != '=')
        return 0;
    pos++; // skip '='

    char buffer[BUFFER_SIZE] = {0};
    int i = 0;
    while (*pos && *pos != '&' && i < BUFFER_SIZE - 1) {
        buffer[i++] = *pos++;
    }
    buffer[i] = '\0';

    // URL-decode the parameter value
    char decoded[BUFFER_SIZE] = {0};
    url_decode(buffer, decoded);

    strncpy(value, decoded, max_len - 1);
    value[max_len - 1] = '\0';
    return 1;
}

int main(void) {
    // Print the HTTP header required for CGI
    printf("Content-Type: text/html\n\n");

    // Check for process management actions from the query string
    char *query_string = getenv("QUERY_STRING");
    if (query_string && strlen(query_string) > 0) {
        char action[50] = {0};
        char pid[50] = {0};
        
        if (get_query_param(query_string, "action", action, sizeof(action)) &&
            get_query_param(query_string, "pid", pid, sizeof(pid))) {
            
            if (strcmp(action, "kill") == 0) {
                // Build the command to kill the process using taskkill on Windows
                char cmd[BUFFER_SIZE] = {0};
                snprintf(cmd, sizeof(cmd), "taskkill /PID %s /F", pid);
                printf("<p class=\"text-red-600 font-bold\">Attempting to kill process with PID %s...</p>\n", pid);
                printf("<pre class=\"bg-gray-200 p-4 rounded\">\n");
                run_command(cmd);
                printf("</pre>\n");
            }
        }
    }

    // Begin HTML output for the dashboard
    printf("<!DOCTYPE html>\n");
    printf("<html lang=\"en\">\n");
    printf("<head>\n");
    printf("    <meta charset=\"UTF-8\">\n");
    printf("    <title>Real-Time Process Monitoring Dashboard</title>\n");
    // Refresh every 20 seconds
    printf("    <meta http-equiv=\"refresh\" content=\"20\">\n");
    printf("    <link href=\"https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css\" rel=\"stylesheet\">\n");
    printf("</head>\n");
    printf("<body class=\"bg-gray-100 font-sans\">\n");
    printf("  <div class=\"container mx-auto p-6 bg-white rounded shadow-md\">\n");
    printf("    <h1 class=\"text-2xl font-bold text-gray-800\">Real-Time Process Monitoring Dashboard</h1>\n");

    // Process Management Section
    printf("    <h2 class=\"text-xl font-semibold text-gray-700 mt-4\">Process Management</h2>\n");
    printf("    <form method=\"get\" action=\"process_monitor.cgi\" class=\"mt-4\">\n");
    printf("      <label for=\"pid\" class=\"block text-gray-600\">Enter Process ID (PID) to kill:</label>\n");
    printf("      <input type=\"text\" id=\"pid\" name=\"pid\" required class=\"border border-gray-300 p-2 rounded w-full mt-1\">\n");
    // Hidden field to indicate the action
    printf("      <input type=\"hidden\" name=\"action\" value=\"kill\">\n");
    printf("      <input type=\"submit\" value=\"Kill Process\" class=\"bg-red-500 text-white p-2 rounded mt-2 hover:bg-red-600\">\n");
    printf("    </form>\n");

    // Display Process List using tasklist
    printf("    <h2 class=\"text-xl font-semibold text-gray-700 mt-4\">Processes</h2>\n");
    printf("    <pre class=\"bg-gray-200 p-4 rounded\">\n");
    run_command("tasklist");
    printf("    </pre>\n");

    // Display CPU Load using WMIC
    printf("    <h2 class=\"text-xl font-semibold text-gray-700 mt-4\">CPU Load Percentage</h2>\n");
    printf("    <pre class=\"bg-gray-200 p-4 rounded\">\n");
    run_command("wmic cpu get loadpercentage");
    printf("    </pre>\n");

    // Display Memory Information using WMIC
    printf("    <h2 class=\"text-xl font-semibold text-gray-700 mt-4\">Memory Information</h2>\n");
    printf("    <pre class=\"bg-gray-200 p-4 rounded\">\n");
    run_command("wmic OS get FreePhysicalMemory,TotalVisibleMemorySize /Format:List");
    printf("    </pre>\n");

    printf("  </div>\n");
    printf("</body>\n");
    printf("</html>\n");

    return 0;
}
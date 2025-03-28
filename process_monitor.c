#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 1024

typedef struct {
    char name[100];
    int memory;
} Process;

Process processes[MAX_PROCESSES];
int process_count = 0;

void print_header() {
    printf("Content-Type: text/html\r\n\r\n");
    printf("<!DOCTYPE html>\n<html lang='en'>\n<head>\n<title>Process Monitor</title>\n");
    printf("<script src='https://cdn.tailwindcss.com'></script>\n");
    printf("<script>\n");
    printf("document.addEventListener('DOMContentLoaded', function () {\n");

    // Filtering Logic
    printf("  document.getElementById('search').addEventListener('input', function () {\n");
    printf("    let filter = this.value.toLowerCase();\n");
    printf("    document.querySelectorAll('.process-row').forEach(row => {\n");
    printf("      let name = row.getAttribute('data-name').toLowerCase();\n");
    printf("      row.style.display = name.includes(filter) ? '' : 'none';\n");
    printf("    });\n");
    printf("  });\n");

    // Sorting Logic
    printf("  document.querySelectorAll('.sortable').forEach(header => {\n");
    printf("    header.addEventListener('click', function () {\n");
    printf("      let column = this.getAttribute('data-column');\n");
    printf("      let table = document.getElementById('process-table');\n");
    printf("      let rows = Array.from(table.rows).slice(1);\n");
    printf("      let ascending = this.getAttribute('data-order') === 'asc';\n");
    printf("      rows.sort((a, b) => {\n");
    printf("        let valA = a.cells[column].innerText;\n");
    printf("        let valB = b.cells[column].innerText;\n");
    printf("        return ascending ? valA.localeCompare(valB, undefined, { numeric: true }) : valB.localeCompare(valA, undefined, { numeric: true });\n");
    printf("      });\n");
    printf("      rows.forEach(row => table.appendChild(row));\n");
    printf("      this.setAttribute('data-order', ascending ? 'desc' : 'asc');\n");
    printf("    });\n");
    printf("  });\n");

    // Right-click context menu for process kill
    printf("  document.querySelectorAll('.process-row').forEach(row => {\n");
    printf("    row.addEventListener('contextmenu', function (event) {\n");
    printf("      event.preventDefault();\n");
    printf("      let processName = this.getAttribute('data-name');\n");
    printf("      let contextMenu = document.getElementById('context-menu');\n");
    printf("      contextMenu.style.display = 'block';\n");
    printf("      contextMenu.style.left = event.pageX + 'px';\n");
    printf("      contextMenu.style.top = event.pageY + 'px';\n");
    printf("      document.getElementById('kill-btn').setAttribute('data-name', processName);\n");
    printf("    });\n");
    printf("  });\n");

    // Hide context menu on click elsewhere
    printf("  document.addEventListener('click', function () {\n");
    printf("    document.getElementById('context-menu').style.display = 'none';\n");
    printf("  });\n");

    // Process kill function
    printf("  document.getElementById('kill-btn').addEventListener('click', function () {\n");
    printf("    let name = this.getAttribute('data-name');\n");
    printf("    window.location.href = '?kill=' + name;\n");
    printf("  });\n");

    printf("});\n");
    printf("</script>\n");

    // Tailwind styling
    printf("<style>\n");
    printf(".hover-effect:hover { background-color: #f3f4f6; }\n");
    printf("#context-menu {\n");
    printf("  display: none; position: absolute; background: white; border: 1px solid #ccc;\n");
    printf("  padding: 8px; box-shadow: 2px 2px 10px rgba(0,0,0,0.1);\n");
    printf("}\n");
    printf("</style>\n");

    printf("</head>\n<body class='bg-gray-100 p-6'>\n");
    printf("<div class='max-w-4xl mx-auto bg-white shadow-lg rounded-lg p-6'>\n");
    printf("<h2 class='text-2xl font-bold mb-4 text-center'>Real-Time Process Monitoring</h2>\n");

    // Search bar
    printf("<input type='text' id='search' placeholder='Search process...' class='border p-2 w-full mb-4'>\n");
}

void print_footer() {
    printf("<div id='context-menu'>\n");
    printf("<button id='kill-btn' class='bg-red-500 text-white px-4 py-2 rounded cursor-pointer'>Kill Process</button>\n");
    printf("</div>\n");
    printf("</div>\n</body>\n</html>\n");
}

void get_processes() {
    FILE *fp = popen("wmic process get Name,WorkingSetSize", "r");
    if (fp == NULL) {
        printf("<p class='text-red-500'>Error fetching processes.</p>\n");
        return;
    }

    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);

    process_count = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        char name[100];
        int memory = 0;
        sscanf(buffer, "%99s %d", name, &memory);
        if (strlen(name) > 0) {
            strcpy(processes[process_count].name, name);
            processes[process_count].memory = memory / 1024; // Convert bytes to KB
            process_count++;
        }
    }
    pclose(fp);

    printf("<table class='table-auto w-full border-collapse border border-gray-300' id='process-table'>\n");
    printf("<thead>\n<tr class='bg-gray-200'>\n");
    printf("<th class='border p-2 sortable' data-column='0' data-order='asc'>Process Name</th>\n");
    printf("<th class='border p-2 sortable' data-column='1' data-order='asc'>Memory (KB)</th>\n");
    printf("</tr>\n</thead>\n<tbody>\n");

    for (int i = 0; i < process_count; i++) {
        printf("<tr class='process-row hover-effect text-center' data-name='%s'>\n", processes[i].name);
        printf("<td class='border p-2'>%s</td>\n", processes[i].name);
        printf("<td class='border p-2'>%d KB</td>\n", processes[i].memory);
        printf("</tr>\n");
    }

    printf("</tbody>\n</table>\n");
}

void kill_process() {
    char *query_string = getenv("QUERY_STRING");
    if (query_string && strstr(query_string, "kill=")) {
        char name[100];
        sscanf(query_string + 5, "%99s", name);
        if (strlen(name) > 0) {
            char command[200];
            sprintf(command, "taskkill /IM %s /F", name);
            system(command);
            printf("<p class='text-green-500'>Process '%s' has been terminated.</p>\n", name);
        } else {
            printf("<p class='text-red-500'>Invalid process name.</p>\n");
        }
    }
}

int main() {
    print_header();
    get_processes();
    kill_process();
    print_footer();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_RECORDS 6608  // Maximum number of records to process

/* Data structures to hold student records */
typedef struct {
    int record_id;
    int hours_studied;
    int attendance;
    int tutoring_sessions;
    int exam_score;
} CurricularData;

typedef struct {
    int extracurricular_activities;  // Boolean: 1 for Yes, 0 for No
    int physical_activity;
    int record_ID;
    int sleep_hours;
} ExtracurricularData;


/**
 * Removes leading and trailing whitespace from a string
 * 
 * @param str The string to trim
 * @return Pointer to the trimmed string
 */
char* trim_whitespace(char *str) {
    if (str == NULL) return str;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;  // All spaces
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    return str;
}

/**
 * Reads and parses a CSV file containing curricular data
 * 
 * @param filename Path to the CSV file
 * @param data Array to store the parsed data
 * @return Number of records read, or -1 on error
 */
int read_csv_file(const char *filename, CurricularData data[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return -1;
    }

    char buffer[256];
    fgets(buffer, sizeof(buffer), file);  // Skip header line

    int index = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL && index < MAX_RECORDS) {
        sscanf(buffer, "%d,%d,%d,%d,%d", 
               &data[index].record_id, 
               &data[index].hours_studied, 
               &data[index].attendance, 
               &data[index].tutoring_sessions, 
               &data[index].exam_score);
        index++;
    }

    fclose(file);
    return index;
}

/**
 * Processes a single line from the YAML file
 * 
 * @param line The line to process
 * @param record Pointer to the record to update
 */
void process_yaml_line(char *line, ExtracurricularData *record) {
    char *colon = strchr(line, ':');
    if (!colon) return;

    *colon = '\0';
    char *key = trim_whitespace(line);
    char *value_str = trim_whitespace(colon + 1);

    if (strcmp(key, "Extracurricular_Activities") == 0) {
        size_t len = strlen(value_str);
        // Remove surrounding quotes if present
        if (len >= 2 && value_str[0] == '\'' && value_str[len-1] == '\'') {
            memmove(value_str, value_str + 1, len - 1);
            value_str[len - 2] = '\0';
            value_str = trim_whitespace(value_str);
        }
        record->extracurricular_activities = (strcmp(value_str, "Yes") == 0) ? 1 : 0;
    } else {
        int value = atoi(value_str);
        if (strcmp(key, "Record_ID") == 0) {
            record->record_ID = value;
        } else if (strcmp(key, "Sleep_Hours") == 0) {
            record->sleep_hours = value;
        } else if (strcmp(key, "Physical_Activity") == 0) {
            record->physical_activity = value;
        }
    }
}

/**
 * Reads and parses a YAML file containing extracurricular data
 * 
 * @param filename Path to the YAML file
 * @param data Array to store the parsed data
 * @return Number of records read, or -1 on error
 */
int read_yaml_file(const char *filename, ExtracurricularData data[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return -1;
    }

    char buffer[256];
    int current_record = -1;
    int found_records = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        trim_whitespace(buffer);
        if (buffer[0] == '\0') continue;

        // Skip the "records:" line
        if (strcmp(buffer, "records:") == 0) {
            found_records = 1;
            continue;
        }

        // Check if this is a new record
        if (strncmp(buffer, "- ", 2) == 0) {
            current_record++;
            if (current_record >= MAX_RECORDS) break;

            // Process the rest of the line after '- '
            char *rest = buffer + 2;
            trim_whitespace(rest);
            if (*rest != '\0') {
                process_yaml_line(rest, &data[current_record]);
            }
        } else if (current_record >= 0) {  // We're inside a record
            process_yaml_line(buffer, &data[current_record]);
        }
    }

    fclose(file);
    return current_record + 1;
}

/**
 * Finds an extracurricular record by Record_ID
 * 
 * @param record_id The Record_ID to search for
 * @param yaml_data Array of extracurricular data
 * @param num_yaml Number of records in yaml_data
 * @return Pointer to the matching record, or NULL if not found
 */
ExtracurricularData* find_extracurricular(int record_id, ExtracurricularData *yaml_data, int num_yaml) {
    for (int i = 0; i < num_yaml; i++) {
        if (yaml_data[i].record_ID == record_id) {
            return &yaml_data[i];
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Validate command line arguments
    if (argc != 2 || strncmp(argv[1], "--TASK=", 7) != 0) {
        printf("Usage: %s --TASK=\"<task_number>\"\n", argv[0]);
        return 1;
    }

    // Parse task number
    int task;
    sscanf(argv[1], "--TASK=%d", &task);

    // Initialize data structures
    CurricularData csv_data[MAX_RECORDS];
    ExtracurricularData yaml_data[MAX_RECORDS];

    // Read input files
    int num_csv = read_csv_file("data/a1-data-curricular.csv", csv_data);
    if (num_csv < 0) return 1;

    int num_yaml = read_yaml_file("data/a1-data-extracurricular.yaml", yaml_data);
    if (num_yaml < 0) return 1;

    // Open output file
    FILE *output = fopen("output.csv", "w");
    if (output == NULL) {
        printf("Error: Could not create output file\n");
        return 1;
    }

    // Process according to task number
    switch (task) {
        case 1: {
            // Students who scored above 90
            fprintf(output, "Record_ID,Exam_Score\n");
            for (int i = 0; i < num_csv; i++) {
                if (csv_data[i].exam_score > 90) {
                    fprintf(output, "%d,%d\n", 
                            csv_data[i].record_id, 
                            csv_data[i].exam_score);
                }
            }
            break;
        }
        case 2: {
            // All extracurricular records
            fprintf(output, "Extracurricular_Activities,Physical_Activity,Record_ID,Sleep_Hours\n");
            for (int i = 0; i < num_yaml; i++) {
                const char *activity = yaml_data[i].extracurricular_activities ? "Yes" : "No";
                fprintf(output, "%s,%d,%d,%d\n",
                        activity,
                        yaml_data[i].physical_activity,
                        yaml_data[i].record_ID,
                        yaml_data[i].sleep_hours);
            }
            break;
        }
        case 3: {
            // Merged data for students scoring above 90
            fprintf(output, "Record_ID,Hours_Studied,Attendance,Tutoring_Sessions,Exam_Score,"
                    "Extracurricular_Activities,Physical_Activity,Sleep_Hours\n");
            for (int i = 0; i < num_csv; i++) {
                if (csv_data[i].exam_score > 90) {
                    ExtracurricularData *ext = find_extracurricular(csv_data[i].record_id, yaml_data, num_yaml);
                    if (ext) {
                        const char *activity = ext->extracurricular_activities ? "Yes" : "No";
                        fprintf(output, "%d,%d,%d,%d,%d,%s,%d,%d\n",
                                csv_data[i].record_id,
                                csv_data[i].hours_studied,
                                csv_data[i].attendance,
                                csv_data[i].tutoring_sessions,
                                csv_data[i].exam_score,
                                activity,
                                ext->physical_activity,
                                ext->sleep_hours);
                    }
                }
            }
            break;
        }
        case 4: {
            // Students with 100% attendance
            fprintf(output, "Record_ID,Exam_Score\n");
            for (int i = 0; i < num_csv; i++) {
                if (csv_data[i].attendance == 100) {
                    fprintf(output, "%d,%d\n", 
                            csv_data[i].record_id, 
                            csv_data[i].exam_score);
                }
            }
            break;
        }
        case 5: {
            // Students who sleep more than or equal to study hours
            fprintf(output, "Record_ID,Exam_Score\n");
            for (int i = 0; i < num_csv; i++) {
                ExtracurricularData *ext = find_extracurricular(csv_data[i].record_id, yaml_data, num_yaml);
                if (ext && ext->sleep_hours >= csv_data[i].hours_studied) {
                    fprintf(output, "%d,%d\n", 
                            csv_data[i].record_id, 
                            csv_data[i].exam_score);
                }
            }
            break;
        }
        case 6: {
            // Students who scored below 60
            fprintf(output, "Record_ID,Exam_Score,Extracurricular_Activities\n");
            for (int i = 0; i < num_csv; i++) {
                if (csv_data[i].exam_score < 60) {
                    ExtracurricularData *ext = find_extracurricular(csv_data[i].record_id, yaml_data, num_yaml);
                    if (ext) {
                        const char *activity = ext->extracurricular_activities ? "Yes" : "No";
                        fprintf(output, "%d,%d,%s\n",
                                csv_data[i].record_id,
                                csv_data[i].exam_score,
                                activity);
                    }
                }
            }
            break;
        }
        default:
            printf("Error: Invalid task number\n");
            fclose(output);
            return 1;
    }

    fclose(output);
    return 0;
}
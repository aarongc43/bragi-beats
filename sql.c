//BRAGI BEATS MYSQL

#include <mysql.h>
#include <stdio.h>


int init_db(void) {

// MySQL connection variables
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    // Initialize Bragi Beats DB library
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize Bragi Beats DB library\n");
        return 1;
    }

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Failed to initialize MySQL connection object\n");
        return 1;
    }

  // Establish connection to MySQL server
    if (!mysql_real_connect(conn, "localhost", "Username", "PasswordHash", "/Users/caseysharp/Desktop/Bragi\ Beats/BragiBeatsDB.sql", 3306, NULL, 0)) {
        fprintf(stderr, "Failed to connect to MySQL server: Error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    // Execute SQL query
    if (mysql_query(conn, "SELECT * FROM `Users Table`")) {
        fprintf(stderr, "Failed to execute SQL query: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    // Store result set
    res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "Failed to store result set\n");
        mysql_close(conn);
        return 1;
    }

    // Print results
    printf("Results:\n");
    while ((row = mysql_fetch_row(res)) != NULL) {
        printf("%s\t%s\t%s\n", row[0], row[1], row[2]); // Assuming 3 columns, adjust as per your schema
    }

    // Free result set
    mysql_free_result(res);

    // Close connection
    mysql_close(conn);

    // Cleanup Bragi Beats DB library
    mysql_library_end();
    
}
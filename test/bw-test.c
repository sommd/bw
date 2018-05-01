#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <check.h>

Suite *create_utils_suite();

int main() {
    // Seed rand
    srand(time(NULL));
    
    // Create suites
    Suite *suites[] = {
        create_utils_suite(),
    };
    
    // Create runner
    SRunner *runner = srunner_create(suites[0]);
    for (int i = 1; i < sizeof(suites) / sizeof(*suites); i++) {
        srunner_add_suite(runner, suites[i]);
    }
    
    // Run
    srunner_run_all(runner, CK_NORMAL);
    int failed = srunner_ntests_failed(runner);
    
    // Exit
    srunner_free(runner);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

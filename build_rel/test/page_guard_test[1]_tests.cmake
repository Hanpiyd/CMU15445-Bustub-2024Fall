add_test( PageGuardTest.SampleTest /home/jyh/bustub-private/build_rel/test/page_guard_test [==[--gtest_filter=PageGuardTest.SampleTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/page_guard_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( PageGuardTest.SampleTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
set( page_guard_test_TESTS PageGuardTest.SampleTest)

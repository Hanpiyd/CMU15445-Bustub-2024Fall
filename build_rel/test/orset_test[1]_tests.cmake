add_test( ORSetTest.AddRemoveTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetTest.AddRemoveTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetTest.AddRemoveTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
add_test( ORSetTest.MergeTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetTest.MergeTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetTest.MergeTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
add_test( ORSetTest.AddWinsTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetTest.AddWinsTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetTest.AddWinsTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
add_test( ORSetTest.DoubleMergeTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetTest.DoubleMergeTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetTest.DoubleMergeTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
add_test( ORSetDriverTest.AddRemoveTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetDriverTest.AddRemoveTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetDriverTest.AddRemoveTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
add_test( ORSetDriverTest.MergeTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetDriverTest.MergeTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetDriverTest.MergeTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
add_test( ORSetDriverTest.AddBackAgainTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetDriverTest.AddBackAgainTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetDriverTest.AddBackAgainTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
add_test( ORSetDriverTest.AddWinsALotTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetDriverTest.AddWinsALotTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetDriverTest.AddWinsALotTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
add_test( ORSetDriverTest.NetworkLostTest /home/jyh/bustub-private/build_rel/test/orset_test [==[--gtest_filter=ORSetDriverTest.NetworkLostTest]==] --gtest_also_run_disabled_tests [==[--gtest_output=xml:/home/jyh/bustub-private/build_rel/test/orset_test.xml]==] [==[--gtest_catch_exceptions=0]==])
set_tests_properties( ORSetDriverTest.NetworkLostTest PROPERTIES WORKING_DIRECTORY /home/jyh/bustub-private/build_rel/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==] TIMEOUT 120)
set( orset_test_TESTS ORSetTest.AddRemoveTest ORSetTest.MergeTest ORSetTest.AddWinsTest ORSetTest.DoubleMergeTest ORSetDriverTest.AddRemoveTest ORSetDriverTest.MergeTest ORSetDriverTest.AddBackAgainTest ORSetDriverTest.AddWinsALotTest ORSetDriverTest.NetworkLostTest)

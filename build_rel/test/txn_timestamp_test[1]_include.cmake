if(EXISTS "/home/jyh/bustub-private/build_rel/test/txn_timestamp_test[1]_tests.cmake")
  include("/home/jyh/bustub-private/build_rel/test/txn_timestamp_test[1]_tests.cmake")
else()
  add_test(txn_timestamp_test_NOT_BUILT txn_timestamp_test_NOT_BUILT)
endif()

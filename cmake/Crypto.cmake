

SET(CRYPTO_LIST)
SET(NUM_NODES "2")
SET(NUM_USERS "1")

FOREACH(X RANGE ${NUM_NODES})
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/keys/node${X}/client.cert)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/keys/node${X}/client.key)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/keys/node${X}/server.cert)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/keys/node${X}/server.key)
ENDFOREACH()

FOREACH(X RANGE ${NUM_USERS})
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/keys/user${X}/client.cert)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/keys/user${X}/client.key)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/keys/user${X}/server.cert)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/keys/user${X}/server.key)
ENDFOREACH()

# test target
add_custom_target(openssl_samples
  SOURCES ${CRYPTO_LIST}
)

# generated keys and certificates for testing
add_custom_command(
  OUTPUT ${CRYPTO_LIST}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
  COMMAND ${PROJECT_SOURCE_DIR}/tests/scripts/gen-crypto.sh ${NUM_NODES} ${NUM_USERS}
)
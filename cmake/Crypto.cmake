SET(CRYPTO_LIST)
SET(NUM_NODES "2")

FOREACH(X RANGE ${NUM_NODES})
  # list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/config/node${X}/client.cert)
  # list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/config/node${X}/client.key)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/config/node${X}/server.cert)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/config/node${X}/server.key)
  list(APPEND CRYPTO_LIST ${PROJECT_SOURCE_DIR}/tests/config/node${X}/settings.yaml)
ENDFOREACH()

# test target
add_custom_target(openssl_samples
  SOURCES ${CRYPTO_LIST}
)

# generated keys and certificates for testing
add_custom_command(
  OUTPUT ${CRYPTO_LIST}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
  COMMAND ${PROJECT_SOURCE_DIR}/tests/scripts/gen-network.sh ${NUM_NODES}
)

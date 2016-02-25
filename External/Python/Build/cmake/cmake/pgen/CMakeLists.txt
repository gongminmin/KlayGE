cmake_minimum_required(VERSION 2.6)

add_definitions(-DNDEBUG)

set(PGEN_SOURCES
    ${SRC_DIR}/Objects/obmalloc.c
    ${SRC_DIR}/Parser/acceler.c
    ${SRC_DIR}/Parser/bitset.c
    ${SRC_DIR}/Parser/firstsets.c
    ${SRC_DIR}/Parser/grammar1.c
    ${SRC_DIR}/Parser/grammar.c
    ${SRC_DIR}/Parser/listnode.c
    ${SRC_DIR}/Parser/metagrammar.c
    ${SRC_DIR}/Parser/node.c
    ${SRC_DIR}/Parser/parser.c
    ${SRC_DIR}/Parser/parsetok.c
    ${SRC_DIR}/Parser/pgen.c
    ${SRC_DIR}/Parser/pgenmain.c
    ${SRC_DIR}/Parser/printgrammar.c
    ${SRC_DIR}/Parser/tokenizer_pgen.c
    ${SRC_DIR}/Python/mysnprintf.c
    ${SRC_DIR}/Python/pyctype.c
)

add_executable(pgen ${PGEN_SOURCES})

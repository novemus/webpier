function(convert_to_cpp_array BINARY_FILE HEADER_FILE ARRAY_NAME)
    if(NOT EXISTS ${HEADER_FILE})
        file(READ ${BINARY_FILE} FILE_CONTENT HEX)

        set(CPP_ARRAY "#pragma once\n\nstatic const unsigned char ${ARRAY_NAME}[] = {\n    ")

        string(LENGTH ${FILE_CONTENT} FILE_LENGTH)
        set(BREAK 44)

        foreach(POS RANGE 0 ${FILE_LENGTH} 2)
            string(SUBSTRING ${FILE_CONTENT} ${POS} 2 BYTE)
            if (BYTE)
                if (${POS} EQUAL ${BREAK})
                    math(EXPR BREAK "${BREAK} + 44" OUTPUT_FORMAT DECIMAL)
                    string(APPEND CPP_ARRAY ",\n    ")
                elseif(${POS} GREATER 0)
                    string(APPEND CPP_ARRAY ", ")
                endif()
                string(APPEND CPP_ARRAY "0x${BYTE}")
            endif()
        endforeach()

        string(APPEND CPP_ARRAY "\n}\;\n")

        file(WRITE ${HEADER_FILE} ${CPP_ARRAY})
    endif()
endfunction()

function(configure_and_generate INPUT_FILE OUTPUT_FILE)
    configure_file(${INPUT_FILE} ${OUTPUT_FILE} @ONLY)
    file(GENERATE OUTPUT ${OUTPUT_FILE} INPUT ${OUTPUT_FILE})
endfunction()

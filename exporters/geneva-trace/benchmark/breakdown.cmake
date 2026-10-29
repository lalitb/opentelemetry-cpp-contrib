# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Add probes only to build-directory copies; never change the SDK or baseline target.
function(mark_stage text_var anchor stage)
  string(FIND "${${text_var}}" "${anchor}" first)
  string(FIND "${${text_var}}" "${anchor}" last REVERSE)
  if(first EQUAL -1 OR NOT first EQUAL last)
    message(FATAL_ERROR "Stage instrumentation needs exactly one match for: ${anchor}")
  endif()
  string(REPLACE "${anchor}" "::end_probe::Switch(::end_probe::Stage::${stage});\n${anchor}"
         marked "${${text_var}}")
  set(${text_var} "${marked}" PARENT_SCOPE)
endfunction()

set(etw_headers "${OPENTELEMETRY_CPP_ROOT}/exporters/etw/include/opentelemetry/exporters/etw")
set(instrumented_headers "${CMAKE_CURRENT_BINARY_DIR}/instrumented/opentelemetry/exporters/etw")
file(MAKE_DIRECTORY "${instrumented_headers}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${etw_headers}/etw_tracer.h" "${etw_headers}/etw_provider.h")
file(READ "${etw_headers}/etw_tracer.h" tracer)
mark_stage(tracer "    Properties evt    = GetSpanAttributes(currentSpan);" AttributeCopy)
mark_stage(tracer "    evt[ETW_FIELD_NAME] = GetName(span);" IdFields)
mark_stage(tracer
           "      evt[ETW_FIELD_STARTTIME] = utils::formatUtcTimestampNsAsISO8601(startTimeNs);"
           Timestamps)
mark_stage(tracer "      evt[ETW_FIELD_DURATION] = endTimeNs - startTimeNs;" FinalFields)
file(WRITE "${instrumented_headers}/etw_tracer.h" "${tracer}")

file(READ "${etw_headers}/etw_provider.h" provider)
mark_stage(provider "    UINT32 eventTags = MICROSOFT_EVENTTAG_NORMAL_PERSISTENCE;" Encoding)
mark_stage(provider "    HRESULT writeResponse = 0;" EtwWrite)
mark_stage(provider "    // Event is larger than ETW max sized of 64KB" Other)
file(WRITE "${instrumented_headers}/etw_provider.h" "${provider}")

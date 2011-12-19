// This code is derived from the revision 68982 from Google Chromium project.
// Most parts of the code are:
//   Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The modifications made to work outside of the Chromium project lead to dropping 
// Thread safe policies, since this is targetting a single thread applications,
// namely the modified version of OpenFOAM 1.7.x, being done by blueCAPE Lda in Portugal.

#include "stack_trace.h"

#include <sstream>
#include <iostream>


class SymbolContext {
 public:
  static SymbolContext* GetInstance() {
    return new SymbolContext();
  }

  // Returns the error code of a failed initialization.
  DWORD init_error() const {
    return init_error_;
  }

  // For the given trace, attempts to resolve the symbols, and output a trace
  // to the ostream os.  The format for each line of the backtrace is:
  //
  //    <tab>SymbolName[0xAddress+Offset] (FileName:LineNo)
  //
  // This function should only be called if Init() has been called.  We do not
  // LOG(FATAL) here because this code is called might be triggered by a
  // LOG(FATAL) itself.
  void OutputTraceToStream(const void* const* trace,
                           int count,
                           std::stringstream* os) {

    char module_name_raw[MAX_PATH];

    for (size_t i = 0; ((int)i < count) && os->good(); ++i) {
      const int kMaxNameLength = 256;
      DWORD_PTR frame = reinterpret_cast<DWORD_PTR>(trace[i]);

      // Code adapted from MSDN example:
      // http://msdn.microsoft.com/en-us/library/ms680578(VS.85).aspx
      ULONG64 buffer[
        (sizeof(SYMBOL_INFO) +
          kMaxNameLength * sizeof(wchar_t) +
          sizeof(ULONG64) - 1) /
        sizeof(ULONG64)];
      memset(buffer, 0, sizeof(buffer));

      // Initialize symbol information retrieval structures.
      DWORD64 sym_displacement = 0;
      PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(&buffer[0]);
      symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
      symbol->MaxNameLen = kMaxNameLength - 1;
      BOOL has_symbol = SymFromAddr(GetCurrentProcess(), frame,
                                    &sym_displacement, symbol);

      // Attempt to retrieve line number information.
      DWORD line_displacement = 0;
      IMAGEHLP_LINE64 line = {};
      line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      BOOL has_line = SymGetLineFromAddr64(GetCurrentProcess(), frame,
                                           &line_displacement, &line);

      // Attempt to retrieve the module to whom the symbol belongs to
      DWORD module_base = SymGetModuleBase(GetCurrentProcess(), frame);

      const char * module_name = "[unknown module]";
      if (module_base && 
        GetModuleFileNameA((HINSTANCE)module_base, module_name_raw, MAX_PATH)) {
        module_name = module_name_raw;
      }

      // Output the backtrace line.
      (*os) << "\t";
      if (has_symbol) {
        (*os) << symbol->Name << " [" << trace[i] << "+"
              << sym_displacement << "]";
      } else {
        // If there is no symbol information, add a spacer.
        (*os) << "(No symbol) [" << trace[i] << "]";
      }
      if (has_line) {
        (*os) << " (" << line.FileName << ":" << line.LineNumber << ")";
      }
      if (module_base) {
        (*os) << "\n\t\t module: " << module_name;
      }
      (*os) << "\n";
    }
  }

 private:

  SymbolContext() : init_error_(ERROR_SUCCESS) {
    // Initializes the symbols for the process.
    // Defer symbol load until they're needed, use undecorated names, and
    // get line numbers.
    SymSetOptions(SYMOPT_DEFERRED_LOADS |
                  SYMOPT_UNDNAME |
                  SYMOPT_LOAD_LINES);
    if (SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
      init_error_ = ERROR_SUCCESS;
    } else {
      init_error_ = GetLastError();
      // TODO(awong): Handle error: SymInitialize can fail with
      // ERROR_INVALID_PARAMETER.
      // When it fails, we should not call debugbreak since it kills the current
      // process (prevents future tests from running or kills the browser
      // process).
      std::cerr << "SymInitialize failed: " << init_error_;
    }
  }

  DWORD init_error_;
};


StackTrace::StackTrace() {
//   typedef USHORT (*CaptureStackBackTraceType)(ULONG, ULONG, PVOID*, PULONG);
//   CaptureStackBackTraceType MyRtlCaptureStackBackTrace = (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace"));

  count_ = 0;
  // When walking our own stack, use CaptureStackBackTrace().
//  if(MyRtlCaptureStackBackTrace!=NULL)
//    count_ = (MyRtlCaptureStackBackTrace)(0, kMaxTraces, trace_, NULL);

  //Prefer it this way
  typedef void (*CaptureContextType)(PCONTEXT);
  CaptureContextType MyRtlCaptureContext = (CaptureContextType)(GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureContext"));
  CONTEXT ContextRecord;

  if(MyRtlCaptureContext!=NULL)
  {
    (MyRtlCaptureContext)(&ContextRecord);
    StackWalkContext(&ContextRecord);
  }
}

StackTrace::StackTrace(EXCEPTION_POINTERS* exception_pointers) {
  // When walking an exception stack, we need to use StackWalk64().
  count_ = 0;
  
  StackWalkContext(exception_pointers->ContextRecord);
}

StackTrace::~StackTrace() {
}

void StackTrace::StackWalkContext(PCONTEXT ContextRecord)
{
  // Initialize stack walking.
  STACKFRAME64 stack_frame;
  memset(&stack_frame, 0, sizeof(stack_frame));
#if defined(_WIN64) || defined(WIN64)
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = ContextRecord->Rip;
  stack_frame.AddrFrame.Offset = ContextRecord->Rbp;
  stack_frame.AddrStack.Offset = ContextRecord->Rsp;
#else
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = ContextRecord->Eip;
  stack_frame.AddrFrame.Offset = ContextRecord->Ebp;
  stack_frame.AddrStack.Offset = ContextRecord->Esp;
#endif
  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;
  while (StackWalk64(machine_type,
                     GetCurrentProcess(),
                     GetCurrentThread(),
                     &stack_frame,
                     ContextRecord,
                     NULL,
                     &SymFunctionTableAccess64,
                     &SymGetModuleBase64,
                     NULL) &&
         count_ < kMaxTraces) {
    trace_[count_++] = reinterpret_cast<void*>(stack_frame.AddrPC.Offset);
  }
}

const void *const *StackTrace::Addresses(size_t* count) {
  *count = count_;
  if (count_)
    return trace_;
  return NULL;
}

void StackTrace::OutputToStream(std::stringstream* os) {
  SymbolContext* context = SymbolContext::GetInstance();
  if(context!=NULL)
  {
    DWORD error = context->init_error();
    if (error != ERROR_SUCCESS) {
      (*os) << "Error initializing symbols (" << error
            << ").  Dumping unresolved backtrace:\n";
      for (int i = 0; (i < count_) && os->good(); ++i) {
        (*os) << "\t" << trace_[i] << "\n";
      }
    } else {
      (*os) << "\n\nBacktrace:\n";
      context->OutputTraceToStream(trace_, count_, os);
    }
    delete context;
    context=NULL;
  }
}


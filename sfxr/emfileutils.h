#ifndef EMFILEUTILS_H
#define EMFILEUTILS_H

#include <emscripten.h>

namespace emfileutils {

using upload_handler = void (*)(const char *filename, const char *type, const void *data, size_t size);

EM_JS(void, upload, (const char *accept, upload_handler callback), {
  // clang-format off
  const input = document.createElement("input");
  input.type = "file";
  input.multiple = false;
  input.accept = UTF8ToString(accept);
  input.style.display = "none";
  input.addEventListener("change", (event) => {
    input.remove();
    const file = event.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (event) => {
        const buffer = new Uint8Array(event.target.result);
        const size = buffer.length * buffer.BYTES_PER_ELEMENT;
        const ptr = Module["_malloc"](size);
        const heap = new Uint8Array(Module["HEAPU8"].buffer, ptr, size);
        heap.set(buffer);
        Module["ccall"](
          "upload_callback_exec",
          null,
          ["string", "string", "number", "number", "number"],
          [file.name, file.type, heap.byteOffset, buffer.length, callback],
        );
        Module["_free"](ptr);
      };
      reader.readAsArrayBuffer(file);
    }
  }, { once: true });
  input.addEventListener("cancel", () => {
    input.remove();
  }, { once: true });
  document.body.appendChild(input);
  input["showPicker"]();
  // clang-format on
});

EM_JS(void, download, (const char *suggested_filename, const char *type, const void *data, size_t size), {
  // clang-format off
  const filename = prompt("File name:", UTF8ToString(suggested_filename));
  if (filename) {
    const blob = new Blob(
      [new Uint8Array(Module["HEAPU8"].buffer, data, size)],
      { type: UTF8ToString(type) },
    );
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = filename;
    link.click();
    setTimeout(() => URL.revokeObjectURL(url), 10000);
  }
  // clang-format on
});

namespace {

extern "C" {
inline void EMSCRIPTEN_KEEPALIVE upload_callback_exec(const char *filename, const char *type, const void *data, size_t size, upload_handler callback) {
  callback(filename, type, data, size);
}
}

} // namespace

} // namespace emfileutils

#endif

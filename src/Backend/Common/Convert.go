package common

/*
#cgo LDFLAGS: -L. -lformat
#include <stdint.h>
#include <stddef.h>

int32_t zig_format_bytes(uint64_t bytes, char* out_buf, size_t out_len);
int32_t zig_format_mb_to_gb(double mb, char* out_buf, size_t out_len);
int32_t zig_format_gb_to_mb(double gb, char* out_buf, size_t out_len);
*/
import "C"
import (
	"fmt"
	"unsafe"
)

func FormatBytes(bytes uint64) (string, error) {
	buf := make([]byte, 32)
	n := C.zig_format_bytes(
		C.uint64_t(bytes),
		(*C.char)(unsafe.Pointer(unsafe.SliceData(buf))),
		C.size_t(len(buf)),
	)
	if n < 0 {
		return "", fmt.Errorf("zig formatting failed")
	}
	return string(buf[:n]), nil
}

func FormatMBToGB(mb float64) (string, error) {
	buf := make([]byte, 32)
	n := C.zig_format_mb_to_gb(
		C.double(mb),
		(*C.char)(unsafe.Pointer(unsafe.SliceData(buf))),
		C.size_t(len(buf)),
	)
	if n < 0 {
		return "", fmt.Errorf("zig formatting failed")
	}
	return string(buf[:n]), nil
}

func FormatGBToMB(gb float64) (string, error) {
	buf := make([]byte, 32)
	n := C.zig_format_gb_to_mb(
		C.double(gb),
		(*C.char)(unsafe.Pointer(unsafe.SliceData(buf))),
		C.size_t(len(buf)),
	)
	if n < 0 {
		return "", fmt.Errorf("zig formatting failed")
	}
	return string(buf[:n]), nil
}
const std = @import("std");

pub fn formatMegabytesToGigabytes(megabytes: f64, buf: []u8) ![]const u8 {
    return std.fmt.bufPrint(buf, "{d:.2} GB", .{megabytes / 1024.0});
}

pub fn formatGigabytesToMegabytes(gigabytes: f64, buf: []u8) ![]const u8 {
    return std.fmt.bufPrint(buf, "{d:.2} MB", .{gigabytes * 1024.0});
}

pub fn formatBytes(bytes: u64, buf: []u8) ![]const u8 {
    const units = [_][]const u8{ "B", "KB", "MB", "GB", "TB" };
    var size = @as(f64, @floatFromInt(bytes));
    var unit_idx: usize = 0;

    while (size >= 1024.0 and unit_idx < units.len - 1) {
        size /= 1024.0;
        unit_idx += 1;
    }

    return std.fmt.bufPrint(buf, "{d:.2} {s}", .{ size, units[unit_idx] });
}

// --- C Export Wrappers ---

export fn zig_format_bytes(bytes: u64, out_buf: [*]u8, out_len: usize) callconv(.c) i32 {
    const result = formatBytes(bytes, out_buf[0..out_len]) catch return -1;
    return @intCast(result.len);
}

export fn zig_format_mb_to_gb(mb: f64, out_buf: [*]u8, out_len: usize) callconv(.c) i32 {
    const result = formatMegabytesToGigabytes(mb, out_buf[0..out_len]) catch return -1;
    return @intCast(result.len);
}

export fn zig_format_gb_to_mb(gb: f64, out_buf: [*]u8, out_len: usize) callconv(.c) i32 {
    const result = formatGigabytesToMegabytes(gb, out_buf[0..out_len]) catch return -1;
    return @intCast(result.len);
}

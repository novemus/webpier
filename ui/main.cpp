#include <wx/wx.h>
#include <wx/taskbar.h>
#include "mainframe.h"
#include "logo.h"

static const unsigned char webpier_logo_png[] = {
0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 
0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x08, 0x06, 0x00, 0x00, 0x00, 0xC3, 0x3E, 0x61, 
0xCB, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0E, 0xC4, 0x00, 0x00, 0x0E, 
0xC4, 0x01, 0x95, 0x2B, 0x0E, 0x1B, 0x00, 0x00, 0x15, 0xEE, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 
0xED, 0x9D, 0x79, 0x78, 0x15, 0xD5, 0xDD, 0xC7, 0x3F, 0x67, 0xE6, 0xAE, 0xD9, 0x49, 0x02, 0x09, 
0x4B, 0x04, 0x62, 0x02, 0x08, 0x22, 0x29, 0xA0, 0x10, 0xA1, 0x14, 0x44, 0xAC, 0xB2, 0x29, 0x45, 
0x69, 0xC1, 0x17, 0x2D, 0x8A, 0xC5, 0xA2, 0x75, 0xA9, 0xAF, 0x4B, 0x71, 0xAB, 0xB6, 0x5A, 0x15, 
0xB5, 0x56, 0xAD, 0xCA, 0xA6, 0xD6, 0xA5, 0x5A, 0x50, 0xAA, 0xC8, 0x62, 0xC0, 0xB7, 0x6A, 0x20, 
0xA2, 0xEC, 0x8B, 0x2C, 0xB2, 0x6F, 0x21, 0xAC, 0x81, 0x40, 0x96, 0xBB, 0xCC, 0xCC, 0x79, 0xFF, 
0xA0, 0x28, 0x21, 0x37, 0xC9, 0xCD, 0xCD, 0xDC, 0x3B, 0xF7, 0x4A, 0x3E, 0xCF, 0xE3, 0xF3, 0xC8, 
0x9D, 0x33, 0xBF, 0xF3, 0xCD, 0x9C, 0xEF, 0xCC, 0x9C, 0x39, 0x2B, 0x34, 0xD1, 0x44, 0x13, 0x4D, 
0x34, 0xD1, 0x44, 0x13, 0xE7, 0x22, 0xC2, 0x6A, 0x01, 0xE1, 0xE0, 0x2D, 0x88, 0xEF, 0xE8, 0xA4, 
0x75, 0xA2, 0x46, 0x8E, 0xEA, 0xA4, 0xBD, 0xAA, 0xD3, 0x5E, 0xB1, 0x91, 0xA9, 0x49, 0xD2, 0x6D, 
0x92, 0x34, 0x45, 0x12, 0x2F, 0x14, 0x9C, 0x42, 0xE0, 0x14, 0x12, 0x43, 0x4A, 0xBC, 0xBA, 0xC4, 
0x27, 0x05, 0x65, 0x7E, 0xC1, 0x51, 0x17, 0x1C, 0xA9, 0x32, 0x28, 0x06, 0x76, 0x7A, 0x34, 0x76, 
0x68, 0x1A, 0xDB, 0x57, 0x42, 0xC9, 0x04, 0xF0, 0x5B, 0xFD, 0xB7, 0x99, 0x4D, 0xCC, 0x1B, 0x60, 
0x26, 0xA8, 0x59, 0x90, 0x95, 0xEC, 0xA0, 0x97, 0x43, 0xA5, 0x8F, 0x02, 0x17, 0x2B, 0x0A, 0xE7, 
0xAB, 0x90, 0x8A, 0x40, 0x35, 0x23, 0x0F, 0x29, 0xF1, 0x49, 0x38, 0xA4, 0x49, 0x36, 0xE9, 0x06, 
0xDF, 0xE8, 0x92, 0x25, 0xA5, 0x5E, 0x56, 0xF7, 0x81, 0x43, 0x66, 0xC4, 0xB7, 0x92, 0x98, 0x34, 
0xC0, 0x1C, 0x88, 0x6B, 0xEB, 0x22, 0x3F, 0x4E, 0x30, 0x58, 0x15, 0x5C, 0x61, 0x13, 0x74, 0x14, 
0x02, 0x7B, 0x24, 0x35, 0x18, 0x06, 0x15, 0x3A, 0xAC, 0xF6, 0x1B, 0x2C, 0xA8, 0xD4, 0x99, 0x7F, 
0xDC, 0xCF, 0xB7, 0x03, 0x40, 0x8B, 0xA4, 0x06, 0x33, 0x88, 0x19, 0x03, 0x7C, 0x0E, 0xB6, 0x64, 
0x1B, 0x97, 0x26, 0xDB, 0x18, 0x6D, 0x53, 0x18, 0xA6, 0x08, 0x5A, 0x21, 0xA2, 0x44, 0xBF, 0xC4, 
0xD0, 0x25, 0x1B, 0xBD, 0x30, 0xBB, 0x54, 0xE3, 0xFD, 0x7C, 0x1F, 0x9B, 0x01, 0x69, 0xB5, 0xAC, 
0x60, 0x88, 0x8E, 0x0B, 0x58, 0x07, 0x05, 0xD0, 0xA2, 0x9D, 0x9B, 0xB1, 0x4E, 0xC1, 0x38, 0x55, 
0xD0, 0x39, 0x6A, 0x0A, 0xBD, 0x36, 0x24, 0x9A, 0xCF, 0xA0, 0xD0, 0xA7, 0x33, 0x7D, 0x81, 0x8F, 
0x8F, 0x7E, 0x0F, 0x55, 0x56, 0x4B, 0xAA, 0x8B, 0xA8, 0xBD, 0x98, 0x2B, 0xE0, 0xFC, 0x14, 0x37, 
0x77, 0xD9, 0x15, 0x6E, 0x50, 0x04, 0x49, 0x56, 0xEB, 0x09, 0x05, 0xCD, 0x60, 0x8F, 0x5F, 0xF2, 
0x72, 0x71, 0x15, 0xD3, 0x06, 0xC0, 0x71, 0xAB, 0xF5, 0x04, 0x22, 0xEA, 0x0C, 0xB0, 0x0C, 0xB2, 
0x53, 0xE3, 0x78, 0xD8, 0x21, 0x18, 0x23, 0x04, 0x0E, 0xAB, 0xF5, 0x98, 0x81, 0x21, 0x29, 0xF5, 
0xEA, 0xBC, 0xB0, 0xDB, 0xC3, 0x8B, 0x83, 0xA0, 0xCC, 0x6A, 0x3D, 0x67, 0x12, 0x35, 0x06, 0x28, 
0x84, 0xE6, 0xAD, 0xDC, 0x3C, 0xE2, 0x50, 0x18, 0x2F, 0x04, 0x2E, 0xAB, 0xF5, 0x98, 0x8E, 0x44, 
0xEA, 0x70, 0xD8, 0xA3, 0xF1, 0xE4, 0x3A, 0x2F, 0xAF, 0x8E, 0x02, 0x9F, 0xD5, 0x92, 0x20, 0x0A, 
0x0C, 0x30, 0x05, 0xEC, 0xFD, 0x9C, 0x4C, 0x88, 0xB7, 0xF1, 0x88, 0x10, 0xA4, 0x47, 0x83, 0xA6, 
0xB0, 0x22, 0x91, 0xBA, 0x64, 0x43, 0x99, 0xCE, 0xBD, 0xDD, 0xBC, 0x7C, 0x6A, 0xB5, 0x1C, 0x4B, 
0x2F, 0xF6, 0x2A, 0x3B, 0xDD, 0x52, 0xEC, 0xBC, 0x66, 0x17, 0xF4, 0x8A, 0xFA, 0xCA, 0x9D, 0xD9, 
0x48, 0x74, 0x8F, 0xC1, 0x3F, 0x4B, 0xAA, 0xB8, 0xA7, 0x1F, 0x1C, 0xB6, 0x4A, 0x86, 0x25, 0x17, 
0x7D, 0x26, 0x38, 0x2E, 0x72, 0x71, 0x5F, 0x9C, 0xCA, 0x24, 0x21, 0x70, 0x5B, 0xA1, 0x21, 0x5A, 
0xD0, 0x25, 0xFB, 0x4F, 0xFA, 0xB9, 0xBD, 0xAB, 0x8F, 0x7F, 0x5B, 0x91, 0x7F, 0xC4, 0x0D, 0xB0, 
0x0C, 0xB2, 0xD3, 0xDC, 0x4C, 0x77, 0x28, 0xF4, 0x3F, 0xE7, 0xEE, 0xFA, 0xDA, 0x90, 0x68, 0x1E, 
0x83, 0xA9, 0x1B, 0xAB, 0x78, 0xE0, 0x6A, 0x38, 0x19, 0xC9, 0xAC, 0x23, 0x5A, 0x00, 0xEB, 0x9D, 
0x0C, 0x49, 0xB4, 0x31, 0x43, 0x15, 0x64, 0x44, 0x32, 0xDF, 0x58, 0xC1, 0x6F, 0xB0, 0xF2, 0x44, 
0x25, 0x63, 0xF2, 0x60, 0x4B, 0xA4, 0xF2, 0x54, 0x22, 0x91, 0xC9, 0x4C, 0x50, 0x37, 0xB9, 0x98, 
0x94, 0x6C, 0xE3, 0xC3, 0xA6, 0xC2, 0xAF, 0x1D, 0xBB, 0x42, 0x8F, 0x94, 0x78, 0x16, 0xAF, 0x77, 
0x32, 0x24, 0x52, 0x79, 0x86, 0xFD, 0x09, 0xF0, 0x39, 0xB8, 0x5A, 0xC7, 0xF1, 0x77, 0xB7, 0x60, 
0x5C, 0xD3, 0x23, 0x3F, 0x38, 0x0C, 0x89, 0xB7, 0x42, 0xE3, 0xEE, 0xCE, 0x5E, 0x5E, 0x23, 0xCC, 
0x4D, 0xCA, 0xA6, 0xF4, 0x96, 0xD5, 0xC6, 0x22, 0x48, 0xCE, 0x8E, 0xE3, 0x43, 0xA7, 0xC2, 0x75, 
0x4D, 0x85, 0x1F, 0x3C, 0x42, 0x60, 0x73, 0x2A, 0x5C, 0x39, 0x51, 0xC5, 0x95, 0xAA, 0xF1, 0xF9, 
0x97, 0x61, 0x34, 0x41, 0xD8, 0x0A, 0x65, 0x26, 0xA4, 0x76, 0x8F, 0x63, 0xAE, 0x53, 0x21, 0x3F, 
0x5C, 0x79, 0x84, 0x1B, 0x89, 0xE0, 0x50, 0xF3, 0x6C, 0xD6, 0x76, 0x1D, 0x82, 0xC7, 0x95, 0xC8, 
0xDE, 0x36, 0x17, 0x52, 0xE9, 0x4E, 0xA9, 0x96, 0x26, 0xF3, 0xD0, 0x56, 0x92, 0xCB, 0x0E, 0x02, 
0x90, 0x7E, 0x64, 0x27, 0xA9, 0xC7, 0xF6, 0xD1, 0x61, 0xDB, 0x62, 0x84, 0x04, 0xD1, 0xD8, 0x72, 
0x93, 0xC8, 0x72, 0x83, 0x57, 0x2E, 0xA8, 0xE2, 0x0E, 0xC0, 0x68, 0x5C, 0xB0, 0xC0, 0x84, 0xC5, 
0x00, 0xB3, 0x21, 0xAD, 0x5B, 0x1C, 0x0B, 0x1D, 0x0A, 0xDD, 0xC3, 0x11, 0x3F, 0xDC, 0x9C, 0x48, 
0x48, 0x67, 0xD1, 0x65, 0xBF, 0x63, 0x45, 0xF7, 0x91, 0x94, 0xA5, 0xB4, 0x0C, 0x29, 0x86, 0xCB, 
0x73, 0x92, 0xAC, 0x7D, 0xEB, 0xC8, 0xD9, 0xBE, 0x94, 0x66, 0xC7, 0xF6, 0x91, 0xBB, 0xFD, 0x2B, 
0x32, 0x0E, 0x6F, 0x6F, 0x78, 0x20, 0x89, 0xAC, 0x90, 0xBC, 0xD1, 0xA9, 0x92, 0x5B, 0x08, 0x83, 
0x09, 0x4C, 0x37, 0xC0, 0x7C, 0x48, 0xEA, 0x18, 0xC7, 0x22, 0x87, 0xE0, 0xE2, 0x58, 0x7C, 0xEC, 
0x2F, 0xC9, 0x1F, 0xCB, 0x07, 0x23, 0x9E, 0xC4, 0xE7, 0x88, 0x33, 0x37, 0xB0, 0x34, 0x70, 0xF8, 
0x3D, 0xE4, 0x6E, 0x2B, 0xA2, 0xC3, 0xB6, 0x25, 0xE4, 0x6E, 0x2B, 0xA2, 0xDD, 0x9E, 0xD5, 0x41, 
0x9E, 0x8B, 0xAC, 0x30, 0x78, 0xB5, 0x53, 0x15, 0xB7, 0x63, 0xF2, 0xEB, 0xC0, 0xD4, 0x02, 0x5A, 
0x01, 0x71, 0x69, 0x6E, 0x3E, 0xB1, 0xA9, 0x5C, 0x66, 0x66, 0xDC, 0x48, 0xE0, 0xB3, 0xBB, 0x98, 
0xF6, 0xEB, 0x37, 0xD8, 0xD0, 0x79, 0x10, 0x88, 0x08, 0xF8, 0x56, 0x4A, 0xEC, 0xFE, 0x2A, 0xF2, 
0xD6, 0xCD, 0x23, 0x77, 0x5B, 0x11, 0x5D, 0x36, 0x7D, 0x46, 0xB3, 0xB2, 0x92, 0x3A, 0xD2, 0x23, 
0x3D, 0x3A, 0x4F, 0xE5, 0x7A, 0x98, 0x64, 0xA6, 0x0C, 0xD3, 0xFE, 0xD2, 0x99, 0xA0, 0xE6, 0xB9, 
0x79, 0xCB, 0xAD, 0x32, 0xC6, 0xAC, 0x98, 0x91, 0xC2, 0x67, 0x77, 0x31, 0x75, 0xDC, 0x3F, 0xD8, 
0xD8, 0xF9, 0x72, 0xCB, 0x34, 0xD8, 0xFC, 0x5E, 0xDA, 0xEE, 0x5D, 0x4D, 0xCF, 0x95, 0x1F, 0xD2, 
0xED, 0xDB, 0x05, 0xA4, 0x04, 0x32, 0x83, 0xC4, 0x38, 0xAE, 0x71, 0x5B, 0xD7, 0x53, 0x5F, 0x07, 
0xA6, 0x60, 0x96, 0x01, 0xC4, 0x16, 0x17, 0x8F, 0xBB, 0x55, 0x1E, 0x8C, 0xC5, 0xC7, 0xFE, 0xFB, 
0x23, 0x9F, 0xA6, 0xF0, 0xA7, 0xE3, 0xAD, 0x96, 0xF1, 0x3D, 0x36, 0xBF, 0x87, 0x76, 0x7B, 0x56, 
0xD1, 0xE7, 0xAB, 0xB7, 0xE8, 0xBA, 0x71, 0x21, 0x71, 0x55, 0x3F, 0xF4, 0x20, 0x4B, 0x89, 0xE7, 
0xA4, 0xC6, 0x2F, 0xBA, 0x78, 0x59, 0x60, 0x46, 0x5E, 0xA6, 0x14, 0xD6, 0x7A, 0x07, 0xA3, 0x52, 
0xEC, 0xBC, 0x8B, 0xC0, 0x66, 0x46, 0xBC, 0x48, 0xB2, 0x35, 0x3B, 0x9F, 0x17, 0x6E, 0xFF, 0x18, 
0xA9, 0x84, 0xF5, 0x8B, 0x38, 0x64, 0xE2, 0x2A, 0x4A, 0xE9, 0xB5, 0x7C, 0x26, 0xF9, 0xCB, 0xDE, 
0xA5, 0xF5, 0xFE, 0x8D, 0x08, 0x40, 0x97, 0x1C, 0x3E, 0x58, 0x41, 0xDF, 0x5E, 0x26, 0xB4, 0x18, 
0x36, 0xDA, 0x00, 0xCB, 0x1C, 0x74, 0xC9, 0xB0, 0x53, 0xA8, 0x08, 0x52, 0x1B, 0x1B, 0x2B, 0xD2, 
0x68, 0xAA, 0x9D, 0xC7, 0xFF, 0xB0, 0x94, 0x23, 0xE9, 0xED, 0xAD, 0x96, 0x12, 0x14, 0x1D, 0xB6, 
0x14, 0x32, 0xA0, 0x70, 0x2A, 0x9D, 0x37, 0xFF, 0x07, 0x7C, 0xDE, 0xE5, 0xEB, 0x2B, 0x19, 0xD8, 
0xD8, 0xBE, 0x83, 0x46, 0xD9, 0xBE, 0x00, 0xE2, 0xCF, 0x73, 0x32, 0x47, 0x55, 0xC8, 0x6E, 0x4C, 
0x1C, 0xAB, 0x28, 0xEC, 0x7B, 0x13, 0xCB, 0x7B, 0x5E, 0x67, 0xB5, 0x8C, 0xA0, 0x39, 0x9A, 0xD6, 
0x96, 0x95, 0xDD, 0x7F, 0xC1, 0xEA, 0x6E, 0xC3, 0x71, 0xFB, 0xCB, 0x5B, 0x67, 0x9F, 0x2C, 0x4E, 
0x99, 0x52, 0xE9, 0x99, 0xDF, 0x98, 0x98, 0x8D, 0x7A, 0x02, 0x6C, 0x75, 0xF3, 0xAC, 0x4B, 0xE5, 
0x9E, 0xC6, 0xC4, 0xB0, 0x0A, 0x4D, 0x75, 0xF0, 0xF0, 0x23, 0xAB, 0x28, 0x4B, 0x0E, 0xED, 0x3B, 
0x3F, 0x1A, 0x70, 0x55, 0x9D, 0xD4, 0x3C, 0xAA, 0xF2, 0x8C, 0xCF, 0x21, 0x9E, 0x2B, 0x18, 0x9F, 
0x55, 0x1A, 0x4A, 0x8C, 0x90, 0x0D, 0xB0, 0xD6, 0xC5, 0xC0, 0x66, 0x2A, 0x0B, 0x22, 0x3D, 0x1E, 
0xDF, 0x2C, 0x96, 0x77, 0x1F, 0xC9, 0x1B, 0x37, 0x4C, 0xB5, 0x5A, 0x86, 0x29, 0x48, 0x28, 0x45, 
0xF0, 0xA2, 0xF4, 0xAA, 0x2F, 0xCE, 0x9B, 0x98, 0x72, 0xAC, 0x21, 0xE7, 0x86, 0xD4, 0x1B, 0x58, 
0x00, 0xF1, 0x49, 0x2A, 0x2F, 0xC7, 0x6A, 0xE1, 0x03, 0x2C, 0xED, 0x15, 0x73, 0x5F, 0xAB, 0xB5, 
0x22, 0x20, 0x55, 0x48, 0xFE, 0x28, 0x1C, 0xC6, 0xB6, 0xA1, 0x33, 0x8E, 0x3E, 0xDA, 0xFF, 0xAF, 
0xC7, 0x52, 0xEA, 0x3F, 0xEB, 0x14, 0x21, 0x19, 0xA0, 0x9D, 0x8B, 0x49, 0x36, 0x41, 0xA7, 0x50, 
0xCE, 0x8D, 0x06, 0xF6, 0x67, 0x76, 0x62, 0x73, 0xC7, 0xFE, 0x56, 0xCB, 0x30, 0x1D, 0x81, 0x4C, 
0x15, 0x92, 0x3F, 0x26, 0x26, 0x1A, 0x3B, 0x87, 0x4E, 0x3F, 0x7A, 0x27, 0x8F, 0xCA, 0x7A, 0xCB, 
0xB7, 0xC1, 0x06, 0x58, 0x0E, 0x1D, 0xDD, 0x2A, 0x77, 0x85, 0x26, 0x31, 0x3A, 0x58, 0x73, 0x51, 
0xC4, 0xBA, 0xDB, 0xAD, 0x22, 0x45, 0xC0, 0x0B, 0xC3, 0xB2, 0x8E, 0x7D, 0x37, 0x74, 0xDA, 0xB1, 
0x1B, 0xFA, 0x3F, 0x2A, 0x6B, 0xFD, 0x3C, 0x6F, 0xB0, 0x01, 0x9A, 0xB9, 0xF9, 0x8B, 0x10, 0x98, 
0xDC, 0x50, 0x1E, 0x39, 0x0C, 0xA1, 0xB2, 0xA2, 0xFB, 0x48, 0xAB, 0x65, 0x44, 0x08, 0x99, 0x23, 
0x84, 0xF1, 0x8F, 0x84, 0xAC, 0x63, 0x8B, 0x86, 0x4C, 0x39, 0x3C, 0x20, 0x50, 0x8A, 0x06, 0x55, 
0x02, 0xD7, 0xBB, 0xE8, 0x9F, 0xA2, 0xF2, 0x99, 0x59, 0xB3, 0x6E, 0xAD, 0xA0, 0xB8, 0x65, 0x67, 
0x9E, 0xB8, 0x7F, 0xB1, 0xD5, 0x32, 0xAC, 0x40, 0x97, 0x88, 0x42, 0x54, 0x75, 0xC2, 0xDC, 0x71, 
0xC9, 0x5B, 0x4F, 0xFF, 0xD8, 0xA0, 0x96, 0xBB, 0x78, 0xC1, 0xA3, 0xB1, 0x5C, 0xF8, 0x60, 0xED, 
0xE3, 0x5F, 0x18, 0x1A, 0x69, 0xA5, 0x7B, 0x01, 0x48, 0x3F, 0xB2, 0x8B, 0xD4, 0x63, 0xFB, 0x82, 
0x3C, 0x51, 0xB0, 0x25, 0xA7, 0x4F, 0xAD, 0x87, 0x0D, 0xA1, 0x52, 0x9A, 0x76, 0x5E, 0x7D, 0x51, 
0x54, 0x81, 0x1C, 0x80, 0xAE, 0xAD, 0x1C, 0xFC, 0x7A, 0xD9, 0xA0, 0xF9, 0x37, 0x25, 0x7F, 0x03, 
0x0D, 0x30, 0xC0, 0x5A, 0x17, 0x03, 0xED, 0x0A, 0x3F, 0x0B, 0x36, 0x7D, 0xB4, 0xB2, 0xB5, 0x8E, 
0x0B, 0x69, 0x36, 0x4E, 0x4F, 0x39, 0xE7, 0xEF, 0xF8, 0x9A, 0xBC, 0xF5, 0xA7, 0x7A, 0xFC, 0x1C, 
0xBE, 0xCA, 0xBA, 0x7B, 0xFC, 0x42, 0xC4, 0x10, 0x0A, 0xC7, 0x53, 0x5A, 0x9D, 0xFA, 0x7F, 0x44, 
0xE5, 0x3F, 0x07, 0x3E, 0xD0, 0x67, 0x53, 0xFE, 0xE8, 0x1A, 0xED, 0x02, 0x42, 0x18, 0x19, 0x86, 
0xA2, 0x76, 0x96, 0x3E, 0xED, 0xFB, 0xD6, 0xC3, 0xA0, 0x5F, 0x01, 0xDB, 0xE3, 0x58, 0xE4, 0x50, 
0xB0, 0xAE, 0xBB, 0xCC, 0x04, 0x8E, 0x27, 0x67, 0xF2, 0xF0, 0xC3, 0xAB, 0xD1, 0x6D, 0xE1, 0x9D, 
0x72, 0x98, 0x7C, 0xBC, 0x84, 0x2B, 0x17, 0x3D, 0x4F, 0x8F, 0x35, 0x1F, 0x91, 0x50, 0x11, 0x52, 
0xFB, 0x4C, 0xE8, 0x48, 0x64, 0xA5, 0xCE, 0x43, 0x1D, 0x3D, 0x3C, 0x19, 0x4C, 0xF2, 0xA0, 0x9E, 
0x00, 0xCB, 0xEC, 0xE4, 0x39, 0x44, 0xEC, 0xF5, 0xF1, 0x9F, 0xCD, 0xAA, 0x6E, 0xC3, 0xC3, 0x5E, 
0xF8, 0xB9, 0x5B, 0x97, 0x30, 0xFE, 0x1F, 0x37, 0x93, 0x58, 0x7E, 0x24, 0xAC, 0xF9, 0xD4, 0x8A, 
0x40, 0x38, 0x15, 0x26, 0xBE, 0x01, 0xCF, 0x8F, 0x03, 0x4F, 0x7D, 0xC9, 0x83, 0xFA, 0x0A, 0x48, 
0xB3, 0x73, 0x27, 0x22, 0x32, 0x43, 0xC8, 0xC3, 0xC9, 0xDA, 0xAE, 0xE1, 0x7D, 0xFF, 0xE7, 0x6E, 
0x5D, 0xC2, 0xC4, 0x69, 0xA3, 0xAD, 0x2B, 0xFC, 0xFF, 0xA2, 0x2A, 0xB4, 0xBE, 0xD4, 0xC5, 0xA8, 
0x60, 0xD2, 0xD6, 0x5B, 0xA8, 0x9F, 0x41, 0x9A, 0x0D, 0xAE, 0x6D, 0xBC, 0x2C, 0x6B, 0xA9, 0x74, 
0x27, 0xB1, 0xAF, 0xF5, 0x85, 0x61, 0x8B, 0x9F, 0xBB, 0xAD, 0x88, 0x89, 0xD3, 0x46, 0xE3, 0xF4, 
0x55, 0x86, 0x2D, 0x8F, 0x86, 0x60, 0x13, 0xDC, 0x22, 0x83, 0x78, 0xC5, 0xD7, 0x6B, 0x80, 0xAC, 
0x38, 0x7E, 0xA5, 0x28, 0x24, 0x98, 0x23, 0xCB, 0x3A, 0xB6, 0xE4, 0xF4, 0xA5, 0x2A, 0x2E, 0xE8, 
0x16, 0xD2, 0x06, 0x91, 0x5C, 0x56, 0xC2, 0x98, 0x7F, 0xDD, 0x1D, 0x35, 0x85, 0x0F, 0xE0, 0x50, 
0xB8, 0x74, 0xB9, 0x83, 0xCE, 0xF5, 0xA5, 0xAB, 0xD3, 0x00, 0x8F, 0x82, 0x62, 0x87, 0x1B, 0xCD, 
0x93, 0x65, 0x1D, 0xE1, 0x7C, 0xFC, 0x5F, 0xB9, 0xE8, 0xAF, 0xA1, 0x8D, 0xF8, 0x0D, 0x27, 0x02, 
0x25, 0x45, 0xE5, 0xD7, 0xF5, 0x25, 0xAB, 0xD3, 0x00, 0x43, 0x21, 0xD7, 0x2E, 0x62, 0x73, 0x68, 
0xF7, 0x99, 0xF8, 0x6D, 0x0E, 0x36, 0x85, 0xA9, 0xED, 0xBF, 0xDD, 0xAE, 0x15, 0xF4, 0x5B, 0xF2, 
0x7A, 0x58, 0x62, 0x37, 0x16, 0x87, 0x60, 0xC4, 0x0A, 0xEA, 0xEE, 0xB0, 0xAB, 0xD3, 0x00, 0x49, 
0x2E, 0x46, 0xC6, 0x7A, 0xC3, 0x0F, 0xC0, 0x8E, 0x76, 0x97, 0x70, 0x22, 0x39, 0xD3, 0xF4, 0xB8, 
0xAA, 0xE6, 0x63, 0xF8, 0xFC, 0x27, 0x1A, 0x3F, 0x01, 0x24, 0x4C, 0xA8, 0x82, 0x6C, 0xC5, 0x46, 
0xCF, 0xBA, 0xD2, 0xD4, 0x65, 0x00, 0xC5, 0xAE, 0x70, 0xB5, 0xC9, 0x9A, 0x2C, 0x21, 0x5C, 0x8D, 
0x3F, 0xDD, 0xD6, 0xCF, 0xA7, 0xD3, 0x96, 0xC2, 0xB0, 0xC4, 0x36, 0x05, 0x81, 0x48, 0xB0, 0x33, 
0xA2, 0xAE, 0x24, 0xB5, 0x1A, 0xE0, 0x2B, 0x68, 0x69, 0x13, 0x74, 0x33, 0x5F, 0x55, 0xE4, 0x59, 
0x1B, 0x86, 0xE6, 0x5F, 0x45, 0xF7, 0x73, 0xE5, 0xA2, 0xE7, 0x4D, 0x8F, 0x6B, 0x36, 0x76, 0xC1, 
0xA0, 0x47, 0xEB, 0x28, 0xE7, 0x5A, 0x0F, 0xA4, 0xBA, 0x18, 0x20, 0x04, 0xCE, 0xF0, 0xC8, 0x8A, 
0x1C, 0xC7, 0x93, 0x32, 0x39, 0xD4, 0xDC, 0xFC, 0x21, 0x8B, 0x79, 0xEB, 0xE6, 0xD1, 0x66, 0xFF, 
0x06, 0xD3, 0xE3, 0x9A, 0x8D, 0x2A, 0xE8, 0x72, 0x05, 0x64, 0xD5, 0x76, 0xBC, 0x56, 0x03, 0xA8, 
0x4A, 0xEC, 0xB7, 0xFC, 0x01, 0xAC, 0xBF, 0xF0, 0x4A, 0xFC, 0x76, 0x73, 0x57, 0xA1, 0x51, 0x74, 
0x8D, 0xAB, 0x16, 0x3E, 0x67, 0x6A, 0xCC, 0x70, 0x21, 0x04, 0xF6, 0x34, 0x17, 0x3F, 0xAD, 0xED, 
0x78, 0x40, 0x03, 0x5C, 0x07, 0xAA, 0x5D, 0xD0, 0x2B, 0x7C, 0xB2, 0x22, 0xC7, 0xB7, 0x17, 0x98, 
0xDF, 0x7D, 0x91, 0xB7, 0x6E, 0x2E, 0xAD, 0x4B, 0x36, 0x9A, 0x1E, 0x37, 0x5C, 0xD8, 0xA0, 0xD6, 
0x4A, 0x50, 0x40, 0x03, 0xDC, 0x09, 0x19, 0x02, 0xCE, 0x0F, 0x9F, 0xA4, 0xC8, 0x50, 0xE5, 0x4C, 
0xE4, 0xBB, 0x0E, 0xFD, 0x4C, 0x8F, 0x3B, 0xB8, 0x60, 0xB2, 0xE9, 0x31, 0xC3, 0x89, 0xAA, 0xD0, 
0x9B, 0x5A, 0x5A, 0x05, 0x03, 0x1A, 0x20, 0xC5, 0x46, 0x57, 0xE5, 0x47, 0xF0, 0xFE, 0xDF, 0x73, 
0x5E, 0x1E, 0x3E, 0x67, 0xBC, 0xA9, 0x31, 0xDB, 0xEF, 0x5C, 0x4E, 0xE6, 0xC1, 0x88, 0x2D, 0xE1, 
0x63, 0x0A, 0x42, 0x70, 0xFE, 0xB7, 0xD0, 0x2C, 0xD0, 0xB1, 0x80, 0x06, 0x48, 0xEB, 0x74, 0x5E, 
0x6F, 0xB3, 0x45, 0xEC, 0x3A, 0xAF, 0x3B, 0xDB, 0xB2, 0x7B, 0xE3, 0xAB, 0xE7, 0x7D, 0x7C, 0xA0, 
0x45, 0x0E, 0xDB, 0xB2, 0x7B, 0x7F, 0xFF, 0x9F, 0xB7, 0x11, 0xD3, 0xB4, 0xD7, 0x84, 0xA1, 0xF5, 
0xEF, 0xAA, 0x85, 0xCF, 0xA2, 0xC8, 0xB0, 0xAC, 0xD5, 0x10, 0x36, 0x54, 0x41, 0x62, 0x8B, 0xE1, 
0xFD, 0x02, 0x76, 0x84, 0x04, 0xEC, 0x0E, 0xD6, 0x4B, 0x8A, 0xDB, 0x99, 0x95, 0xF9, 0xB2, 0xEE, 
0x23, 0xF9, 0xF7, 0xD5, 0x8F, 0x51, 0x96, 0x94, 0x09, 0x42, 0xA0, 0x18, 0x1A, 0xCD, 0x0F, 0xEF, 
0xE0, 0xD2, 0xAF, 0xDF, 0xE1, 0xA7, 0x5F, 0xBD, 0x89, 0xCB, 0x5B, 0x01, 0xC0, 0x37, 0x3D, 0x47, 
0x51, 0x70, 0xF9, 0x9D, 0x1C, 0xC8, 0xE8, 0x00, 0xE2, 0x07, 0x5F, 0xAA, 0x9A, 0x8F, 0xBC, 0x75, 
0xF3, 0x18, 0x50, 0x38, 0x85, 0xEC, 0x5D, 0xCB, 0x1B, 0x94, 0xB7, 0xD9, 0xDF, 0xFF, 0xED, 0x76, 
0xAD, 0x38, 0x35, 0x2D, 0x2B, 0x06, 0xD1, 0x96, 0x2F, 0x6D, 0x1B, 0xE8, 0xF7, 0x80, 0xAD, 0x7C, 
0xF7, 0xB7, 0xCD, 0x78, 0x5E, 0x3F, 0x51, 0x1E, 0xF0, 0x91, 0xD1, 0x50, 0xFC, 0x76, 0x37, 0x52, 
0x51, 0x29, 0x4B, 0xCA, 0xC0, 0xE3, 0x4E, 0x46, 0x0A, 0x85, 0x8A, 0x84, 0x34, 0x36, 0x77, 0x1A, 
0xC0, 0xD2, 0x5E, 0x63, 0xA8, 0x74, 0x25, 0xF1, 0xC1, 0x88, 0x27, 0x58, 0xDC, 0x67, 0x1C, 0xE5, 
0x89, 0xCD, 0x6B, 0xCC, 0xCD, 0x97, 0x8A, 0x4A, 0x49, 0xCB, 0x0B, 0x58, 0x7A, 0xC9, 0x68, 0x0E, 
0xB6, 0xC8, 0xA1, 0xE3, 0xD6, 0xC5, 0xD8, 0x35, 0x6F, 0xBD, 0xF9, 0x1E, 0x6C, 0x9E, 0xC3, 0xBC, 
0xAB, 0xEE, 0xAF, 0x66, 0xA6, 0xC6, 0x72, 0xD5, 0xC2, 0xE7, 0x68, 0xB7, 0x37, 0xC8, 0x45, 0x1D, 
0xA2, 0x0C, 0x67, 0x4E, 0x76, 0xE9, 0xE4, 0xE2, 0xD2, 0x39, 0x67, 0xFF, 0x1E, 0xB0, 0x62, 0xB0, 
0x2F, 0x89, 0x5D, 0xD2, 0x20, 0xA0, 0x63, 0x42, 0x45, 0x53, 0xED, 0x2C, 0xEF, 0x31, 0x92, 0x85, 
0x03, 0xEF, 0xE2, 0x60, 0x46, 0x6E, 0xBD, 0xE9, 0xC7, 0xBC, 0x7F, 0x17, 0xDD, 0xD7, 0xCE, 0xA1, 
0xE0, 0xF2, 0xBB, 0x58, 0x34, 0xF0, 0x8E, 0xEF, 0x7F, 0x4F, 0x3A, 0x71, 0x90, 0xDB, 0xA6, 0x8C, 
0x22, 0xAB, 0xF8, 0xDB, 0x3A, 0xCF, 0x9F, 0xF7, 0xF3, 0xFB, 0x4E, 0x19, 0xC0, 0x24, 0x92, 0x8F, 
0x97, 0xF0, 0xA7, 0x3F, 0xE5, 0x61, 0xD3, 0x63, 0x6E, 0x53, 0x10, 0x00, 0xA4, 0xE0, 0xCD, 0xF3, 
0x4E, 0x32, 0xEE, 0xEC, 0xDF, 0x6B, 0xDC, 0x1E, 0x55, 0xF7, 0xFC, 0xA2, 0xAD, 0xE2, 0x76, 0xB7, 
0x31, 0x5B, 0x80, 0x4D, 0xF7, 0x93, 0xBF, 0xEC, 0x7D, 0x26, 0x4D, 0xEE, 0xC7, 0xF5, 0xEF, 0xDD, 
0x41, 0x5C, 0x65, 0xDD, 0xCB, 0xE7, 0x77, 0xD8, 0x56, 0x44, 0x5C, 0x55, 0x19, 0x97, 0x7D, 0x59, 
0x7D, 0x2D, 0x84, 0x13, 0x49, 0x19, 0xBC, 0xF4, 0xDB, 0x0F, 0xD9, 0xDB, 0xBA, 0x6B, 0x9D, 0xE7, 
0xAF, 0xCA, 0x1B, 0xDE, 0x68, 0xCD, 0x67, 0xD2, 0xAF, 0xE8, 0xF5, 0x98, 0x2D, 0x7C, 0x00, 0x67, 
0xEB, 0x8C, 0x80, 0xEF, 0xC3, 0x1A, 0x06, 0x28, 0x7B, 0x7B, 0xB6, 0xAA, 0x57, 0x56, 0x85, 0xAD, 
0x03, 0xC8, 0xAE, 0xF9, 0xE8, 0xF3, 0xCD, 0xBB, 0x3C, 0xFE, 0xA7, 0xEE, 0xB4, 0xAA, 0xA3, 0x25, 
0x6D, 0xCE, 0x90, 0x49, 0x14, 0xB7, 0xEC, 0xCC, 0xC7, 0x43, 0x1F, 0xAE, 0x71, 0xAC, 0x3C, 0x21, 
0x9D, 0x97, 0x6E, 0x9D, 0x55, 0xAB, 0x09, 0x0E, 0x36, 0x3F, 0x9F, 0x92, 0xCC, 0x8E, 0xA6, 0x69, 
0x16, 0x86, 0x4E, 0x9F, 0xA5, 0x6F, 0x99, 0x16, 0xCF, 0x0A, 0xFC, 0x07, 0x0E, 0x06, 0x1C, 0x0B, 
0x57, 0xC3, 0x00, 0xB6, 0xB8, 0x34, 0x47, 0x24, 0x3A, 0xB7, 0xE2, 0xAA, 0xCA, 0x18, 0x5C, 0xF0, 
0x6C, 0xAD, 0xC7, 0x57, 0xFD, 0x64, 0x04, 0x4F, 0xDC, 0xBF, 0x98, 0xAF, 0x2F, 0x19, 0x1D, 0xF0, 
0x78, 0x79, 0x62, 0xF3, 0x5A, 0x4D, 0xB0, 0x2A, 0xEF, 0x1A, 0x53, 0xDF, 0xFD, 0x79, 0xEB, 0xE6, 
0x91, 0x64, 0xF1, 0x30, 0xAF, 0xC6, 0x22, 0x75, 0xA1, 0xCA, 0x99, 0xD7, 0xD5, 0xB8, 0xB1, 0x6B, 
0x5C, 0x25, 0x77, 0xEF, 0x1E, 0x11, 0x1B, 0x37, 0xDD, 0x7D, 0xED, 0x1C, 0x5A, 0x96, 0x6C, 0x0E, 
0xF9, 0xFC, 0x53, 0x26, 0xF8, 0xA0, 0x9A, 0x09, 0x8E, 0xA6, 0x66, 0x51, 0xD8, 0xB7, 0xC6, 0xAB, 
0x2E, 0x74, 0xA4, 0xA4, 0x7F, 0x61, 0xEC, 0xCF, 0x22, 0x56, 0x13, 0xE3, 0xDB, 0xB0, 0x21, 0xBD, 
0x46, 0xBD, 0xAE, 0xC6, 0x67, 0xA0, 0x67, 0x47, 0x64, 0x9B, 0x38, 0x07, 0x17, 0x3C, 0xC3, 0x8C, 
0x1B, 0x67, 0x84, 0xBC, 0x32, 0x57, 0x79, 0x62, 0x3A, 0x2F, 0xDC, 0xFE, 0x31, 0x9D, 0xBE, 0xFB, 
0x02, 0xBB, 0xDF, 0xC3, 0x9A, 0x6E, 0xC3, 0x4C, 0x5D, 0xE2, 0x2D, 0x6B, 0xDF, 0x3A, 0x72, 0x77, 
0x2C, 0x35, 0x2D, 0x9E, 0x55, 0x18, 0x3E, 0x2F, 0x65, 0x2B, 0x96, 0xD4, 0xF8, 0xBD, 0xC6, 0x13, 
0x40, 0x2F, 0x8D, 0xEC, 0xA3, 0xAE, 0xC7, 0x9A, 0x8F, 0x69, 0x79, 0xE0, 0xBB, 0x46, 0xC5, 0xA8, 
0x72, 0x27, 0xB3, 0x3A, 0xEF, 0x6A, 0x96, 0x5D, 0xFC, 0x4B, 0xD3, 0xD7, 0xF7, 0xEB, 0xBF, 0x78, 
0x9A, 0xA9, 0xF1, 0x2C, 0x43, 0xD3, 0xE0, 0xE0, 0x9E, 0x1A, 0x3F, 0xD7, 0x30, 0x80, 0xBF, 0xB4, 
0xDE, 0xA1, 0xE4, 0xA6, 0x33, 0xE4, 0xD3, 0xA7, 0x23, 0x9E, 0x67, 0x30, 0x34, 0x3B, 0xB6, 0x8F, 
0xEE, 0x6B, 0x3E, 0xB6, 0x5A, 0x86, 0x29, 0x48, 0x5D, 0xE2, 0xD9, 0x53, 0x73, 0xBF, 0xAA, 0xA8, 
0x18, 0xEB, 0xDF, 0x7D, 0xED, 0x1C, 0xB2, 0x77, 0x7C, 0x6D, 0xB5, 0x8C, 0x1A, 0xFC, 0x64, 0xCD, 
0x9C, 0xA8, 0x1A, 0xE9, 0x1B, 0x0E, 0x6A, 0xD4, 0x01, 0x94, 0x78, 0xD6, 0x19, 0x5E, 0x22, 0x7E, 
0x4B, 0xF6, 0x5D, 0xFA, 0x76, 0xC2, 0xF6, 0xF6, 0xBD, 0x27, 0x88, 0xE8, 0x59, 0x6A, 0xCE, 0x9F, 
0xB7, 0x6E, 0xDE, 0x14, 0x24, 0x15, 0x56, 0x0B, 0x31, 0x03, 0x45, 0x05, 0x77, 0x3A, 0xC7, 0xCF, 
0xDE, 0xED, 0x38, 0xAA, 0x16, 0x75, 0x1C, 0x36, 0xBD, 0xF4, 0x2D, 0x90, 0x63, 0xAD, 0xD6, 0x01, 
0x20, 0xA5, 0xFC, 0x6C, 0xEE, 0x2D, 0xE9, 0x83, 0xAC, 0xD6, 0x11, 0x6E, 0xA2, 0xE2, 0x15, 0x70, 
0x1A, 0x4D, 0xCA, 0x49, 0x92, 0xE8, 0xB8, 0xE3, 0x04, 0xBC, 0x6C, 0xB5, 0x86, 0x48, 0x10, 0x55, 
0x06, 0x58, 0x70, 0x4B, 0xDA, 0x3E, 0xA4, 0xFC, 0xB3, 0xD5, 0x3A, 0xA4, 0x94, 0xDB, 0x5D, 0xC9, 
0x69, 0x73, 0xAD, 0xD6, 0x11, 0x09, 0x4C, 0x7B, 0xDF, 0xF6, 0xFF, 0xEB, 0xB1, 0x14, 0x77, 0x82, 
0x91, 0x00, 0xA0, 0x2A, 0xA2, 0x9D, 0x90, 0xA2, 0x3D, 0x80, 0x44, 0x6F, 0x23, 0xA0, 0xC3, 0x0F, 
0x29, 0xC5, 0x25, 0x12, 0x6A, 0x8C, 0xD2, 0xD0, 0x35, 0x06, 0x2F, 0xB8, 0x35, 0x6D, 0x63, 0xF9, 
0xBE, 0xF5, 0xCF, 0x26, 0x66, 0x5D, 0xD4, 0x17, 0x22, 0xB7, 0x6F, 0x4E, 0x00, 0xA6, 0xCF, 0x1A, 
0x25, 0x74, 0x0B, 0xF3, 0x8F, 0x18, 0x0D, 0xAA, 0x03, 0xF4, 0x98, 0xB2, 0xC2, 0xDE, 0x82, 0xAC, 
0xF3, 0x14, 0xC5, 0xD6, 0x5F, 0x11, 0xE2, 0x62, 0x29, 0x0D, 0x37, 0xE2, 0xD4, 0x8E, 0x20, 0x02, 
0xD1, 0x1C, 0x08, 0x79, 0xF2, 0x9D, 0x84, 0xA5, 0xE5, 0x5E, 0xFD, 0x8A, 0x2F, 0x6E, 0x6B, 0x51, 
0x3E, 0x74, 0xCA, 0x89, 0x74, 0x54, 0xFF, 0x6A, 0x01, 0xA6, 0x77, 0x4A, 0xD5, 0x8F, 0xD0, 0xA4, 
0xA6, 0x75, 0x9E, 0x7B, 0x6B, 0x8B, 0xAD, 0xF5, 0xA7, 0x8D, 0x7D, 0x82, 0x32, 0x40, 0xFE, 0xF3, 
0x7B, 0xDD, 0x69, 0xC9, 0x71, 0xBF, 0x45, 0x72, 0x9B, 0x20, 0x7C, 0xCB, 0xC2, 0x4A, 0xF8, 0xDB, 
0xDC, 0xF1, 0xA9, 0x77, 0x83, 0x90, 0xC3, 0xA7, 0x97, 0xF6, 0x91, 0xC8, 0x2F, 0x09, 0xF3, 0xBE, 
0x46, 0x01, 0xF8, 0xFA, 0x93, 0xF1, 0x69, 0x31, 0xBB, 0xCD, 0x4D, 0x43, 0xA9, 0xB7, 0x0E, 0x30, 
0x74, 0xDA, 0xD1, 0x6B, 0xD3, 0x93, 0xE2, 0x76, 0x08, 0xC9, 0x73, 0xE1, 0x2C, 0x7C, 0x00, 0x01, 
0x77, 0x0E, 0x99, 0x76, 0x74, 0x04, 0xC0, 0x9C, 0xF1, 0xA9, 0x45, 0x52, 0x8A, 0x87, 0xC2, 0x99, 
0x5F, 0x20, 0x0C, 0x43, 0x7E, 0x14, 0xE9, 0x3C, 0xAD, 0xA4, 0xEE, 0xC9, 0xA1, 0xD3, 0x8E, 0x3C, 
0x20, 0x04, 0xB3, 0x00, 0xF3, 0x27, 0xD6, 0xD5, 0x82, 0x82, 0x98, 0x3E, 0x64, 0xC6, 0xF1, 0x6C, 
0x00, 0x77, 0x72, 0xB3, 0xC9, 0x52, 0xCA, 0x77, 0x23, 0x95, 0x37, 0x80, 0x44, 0x5A, 0xB2, 0x85, 
0xAB, 0x55, 0xD4, 0x6A, 0x80, 0x21, 0x53, 0x0F, 0xFF, 0x4A, 0x08, 0xF1, 0x97, 0x48, 0x8A, 0x01, 
0x40, 0xD0, 0x4C, 0x48, 0xED, 0xBD, 0xFC, 0xE7, 0xF7, 0xBA, 0x67, 0x8D, 0x12, 0x3A, 0x86, 0xE3, 
0x2E, 0x09, 0x75, 0x0F, 0xFF, 0x31, 0x09, 0x09, 0xBB, 0x0F, 0xCA, 0xF4, 0x9D, 0x91, 0xC8, 0x2B, 
0x5A, 0x08, 0x68, 0x80, 0xE1, 0x33, 0x0E, 0x27, 0x2A, 0x42, 0x79, 0x29, 0xD2, 0x62, 0x4E, 0x23, 
0x10, 0x97, 0xA4, 0x25, 0xBA, 0xA7, 0x01, 0xCC, 0x9D, 0x90, 0x74, 0x04, 0xDD, 0x3E, 0x40, 0xC2, 
0x89, 0x08, 0x64, 0x5D, 0xB9, 0x72, 0x82, 0xF0, 0x47, 0x20, 0x9F, 0xA8, 0x21, 0xA0, 0x01, 0x0C, 
0xD4, 0xEB, 0x11, 0xA4, 0x47, 0x5A, 0x4C, 0x35, 0x84, 0x18, 0x33, 0x74, 0x46, 0xE9, 0xBD, 0x70, 
0xCA, 0x04, 0x86, 0x2E, 0xAF, 0x88, 0x90, 0x09, 0xCE, 0x29, 0x02, 0xB7, 0x03, 0x48, 0x69, 0xF9, 
0xBC, 0x40, 0x01, 0x02, 0xC3, 0x78, 0xEC, 0x99, 0x9B, 0x9E, 0xB7, 0x8F, 0x9E, 0xF9, 0xFB, 0x72, 
0xEE, 0x49, 0x67, 0x55, 0xDE, 0x35, 0xF3, 0x66, 0x8D, 0xF8, 0xF3, 0xE8, 0x70, 0xAD, 0xF1, 0x2F, 
0xA0, 0xD9, 0xFA, 0x56, 0xCD, 0xEF, 0x48, 0x39, 0x71, 0x38, 0x2C, 0xF1, 0xAD, 0x42, 0xF7, 0xB0, 
0xBC, 0x9D, 0x46, 0xC0, 0x41, 0x0D, 0x01, 0x3F, 0x03, 0x87, 0x4E, 0x3F, 0xBA, 0x4B, 0x60, 0xEE, 
0xA8, 0xE0, 0x50, 0xB1, 0xFB, 0xAB, 0xB8, 0xE5, 0xF5, 0x1B, 0xB9, 0x70, 0xD3, 0xFF, 0x01, 0xA7, 
0x36, 0x75, 0x7C, 0x69, 0xE2, 0x6C, 0x8A, 0x5B, 0x75, 0x09, 0x4B, 0x7E, 0x37, 0xBC, 0x3B, 0x91, 
0xDE, 0xCB, 0xFF, 0x15, 0x96, 0xD8, 0x56, 0x51, 0xA1, 0x31, 0xB6, 0x93, 0x87, 0x77, 0x02, 0x1D, 
0xAB, 0xF1, 0x0A, 0x18, 0xFC, 0xF2, 0xFE, 0xB6, 0x02, 0xD1, 0x3A, 0xFC, 0xB2, 0x82, 0xC3, 0x6F, 
0x77, 0x33, 0xF5, 0xA6, 0xB7, 0xD8, 0xD0, 0x69, 0x20, 0x00, 0x49, 0xE5, 0x47, 0xF8, 0xDD, 0x2B, 
0x23, 0xE9, 0xFC, 0x5F, 0x43, 0x98, 0xCD, 0x9C, 0x21, 0x0F, 0x36, 0x6A, 0x36, 0x52, 0xB4, 0x21, 
0x25, 0xFE, 0xA3, 0x1E, 0x6A, 0x5D, 0x1C, 0xB9, 0x46, 0x23, 0x4B, 0xC7, 0x11, 0x0F, 0xF6, 0x11, 
0x22, 0x3A, 0x7A, 0xE4, 0x4E, 0x63, 0xA8, 0x36, 0x56, 0x77, 0x1B, 0x86, 0xAE, 0xDA, 0xC9, 0xDD, 
0x56, 0x84, 0xCB, 0x57, 0x49, 0x8F, 0xD5, 0xFF, 0x46, 0xD5, 0xFD, 0xEC, 0x6C, 0xDB, 0x03, 0x43, 
0x35, 0x6F, 0xDF, 0x0A, 0x8F, 0x2B, 0x89, 0xC4, 0x93, 0x47, 0x68, 0xBF, 0x7B, 0xA5, 0x69, 0x31, 
0xAD, 0x44, 0x97, 0xAC, 0xFF, 0xD0, 0xCF, 0xE4, 0xDA, 0x36, 0xA0, 0xAE, 0x61, 0x80, 0xDC, 0xE1, 
0xF7, 0x5E, 0x2E, 0x84, 0x88, 0xBA, 0x05, 0xF5, 0x0D, 0xD5, 0xC6, 0xD6, 0x9C, 0x3E, 0x1C, 0x6B, 
0xD6, 0x86, 0x0B, 0xBE, 0xFB, 0x02, 0xBB, 0xEE, 0x23, 0x77, 0xFB, 0x52, 0xBA, 0x6E, 0x58, 0xC4, 
0xDE, 0xD6, 0x5D, 0xBF, 0x5F, 0x2B, 0xD7, 0x0C, 0xF6, 0xB4, 0xB9, 0x88, 0x2E, 0x9B, 0x3E, 0xB3, 
0x7C, 0xC1, 0x47, 0x33, 0xF0, 0x1A, 0xBC, 0x7D, 0xAD, 0xC6, 0xC2, 0xDA, 0x8E, 0xD7, 0x78, 0x05, 
0x08, 0x41, 0xDF, 0xF0, 0x4A, 0x6A, 0x1C, 0x4B, 0x7B, 0x5D, 0xCF, 0x53, 0xF7, 0x7C, 0xC6, 0x81, 
0x16, 0x39, 0x00, 0xB4, 0xD9, 0xFF, 0x2D, 0xF7, 0xBC, 0x38, 0x98, 0x9B, 0xDF, 0xBC, 0x89, 0x96, 
0x25, 0x9B, 0x4C, 0xC9, 0xA3, 0x3C, 0xB1, 0x39, 0xEF, 0xFC, 0xEA, 0x6F, 0x54, 0xB8, 0x93, 0x4D, 
0x89, 0x67, 0x19, 0x12, 0x59, 0xEE, 0xA7, 0xCE, 0x86, 0xAD, 0x1A, 0x4F, 0x80, 0x0E, 0xC3, 0xEF, 
0xBF, 0xCF, 0x9A, 0x4E, 0x98, 0xE0, 0x29, 0x4F, 0x48, 0xA7, 0xA8, 0xF7, 0x58, 0x14, 0xA9, 0x93, 
0xB5, 0x6F, 0x3D, 0x76, 0xCD, 0x47, 0xAB, 0x03, 0xDF, 0xD1, 0xAF, 0xE8, 0x0D, 0x5A, 0x1E, 0xD8, 
0x4C, 0x49, 0x66, 0x27, 0x2A, 0xE2, 0x9A, 0x35, 0x6A, 0x6E, 0xC0, 0xF1, 0x94, 0x56, 0x6C, 0xCD, 
0xED, 0x4B, 0xDE, 0xBA, 0x4F, 0x70, 0xF8, 0x23, 0x3F, 0x4E, 0xD2, 0x0C, 0x74, 0xC9, 0x8E, 0x0A, 
0x2F, 0x0F, 0x4E, 0xAD, 0x63, 0xD7, 0xF1, 0x1A, 0x5F, 0x01, 0x43, 0xA7, 0x1F, 0xDD, 0x2F, 0x20, 
0x66, 0xF6, 0x52, 0xCB, 0x3C, 0xB0, 0x99, 0xAB, 0x16, 0x3E, 0x47, 0xCF, 0x55, 0xB3, 0xAB, 0xFD, 
0x31, 0x3B, 0xDB, 0xF6, 0x60, 0xFE, 0xCF, 0xEF, 0x65, 0xC3, 0x05, 0x97, 0x37, 0x6A, 0x33, 0xE8, 
0xF8, 0xF2, 0xA3, 0xDC, 0xF9, 0xCA, 0x88, 0x98, 0x58, 0x0F, 0xE8, 0x6C, 0xAA, 0x74, 0x9E, 0xED, 
0x50, 0xC5, 0xBD, 0x75, 0xA5, 0xA9, 0x76, 0x65, 0x86, 0x4E, 0x29, 0x4E, 0x17, 0xAA, 0x6B, 0x07, 
0x90, 0x18, 0x56, 0x65, 0xF5, 0x60, 0xD3, 0xBC, 0xB4, 0xDD, 0xBD, 0x2A, 0xE0, 0xB1, 0x0E, 0xDB, 
0x8A, 0x10, 0x01, 0xE6, 0xE7, 0x77, 0xD9, 0xF4, 0x59, 0xC0, 0x8A, 0xDB, 0xFE, 0xCC, 0x8E, 0x14, 
0xF6, 0xBD, 0x99, 0xD5, 0xDD, 0x86, 0x73, 0x32, 0xB1, 0x79, 0x68, 0x82, 0xA4, 0x64, 0xC8, 0xA7, 
0x4F, 0x33, 0xB8, 0x60, 0x72, 0x74, 0x8D, 0xA1, 0xAB, 0x0B, 0x89, 0x51, 0xE2, 0xE7, 0xA2, 0x4B, 
0x7C, 0xD4, 0xE9, 0xDC, 0xB3, 0x0C, 0x70, 0xA4, 0x93, 0x50, 0x45, 0xA3, 0x5E, 0xA4, 0x29, 0xC7, 
0x8A, 0xAB, 0xED, 0x7C, 0x9D, 0xB3, 0xE3, 0x6B, 0x14, 0xE3, 0x87, 0x49, 0x95, 0x89, 0x27, 0x8F, 
0xD0, 0xEA, 0x40, 0xF5, 0x2C, 0x72, 0x03, 0x14, 0xAA, 0x6A, 0x98, 0x3B, 0x1E, 0xE3, 0x78, 0x72, 
0x4B, 0x3E, 0x1D, 0x74, 0x37, 0x4B, 0x7B, 0x8D, 0x09, 0x79, 0xD1, 0xA8, 0xAC, 0xBD, 0x6B, 0xB9, 
0xF6, 0xA3, 0x87, 0xC8, 0xDD, 0xFE, 0x95, 0xA9, 0xDA, 0xC2, 0x81, 0x4F, 0x67, 0x49, 0x76, 0x15, 
0xFD, 0x44, 0x2D, 0xB5, 0xFF, 0xD3, 0x54, 0x33, 0xC0, 0xB0, 0xA9, 0x47, 0x2E, 0x46, 0x11, 0xCB, 
0x1A, 0x92, 0x51, 0xCF, 0x95, 0x1F, 0x70, 0xD9, 0x97, 0xAF, 0xD1, 0xEA, 0xBF, 0x53, 0xBC, 0x54, 
0xDD, 0x8F, 0x6A, 0x44, 0xEF, 0x2C, 0xDA, 0xC3, 0x69, 0xED, 0x78, 0x7B, 0xF4, 0x8B, 0x6C, 0x0B, 
0x71, 0xF1, 0x08, 0x61, 0x18, 0x74, 0xD8, 0xB6, 0x84, 0x41, 0xFF, 0x79, 0x91, 0x8E, 0x5B, 0x16, 
0x47, 0xED, 0xDF, 0x5A, 0xA9, 0x71, 0x63, 0x47, 0x0F, 0xF5, 0xCE, 0x68, 0x3D, 0xDB, 0x00, 0x13, 
0x51, 0xC4, 0xDF, 0x83, 0xC9, 0x40, 0xD5, 0xBC, 0xDC, 0xF8, 0xEE, 0x44, 0xBA, 0xAF, 0xF9, 0x18, 
0x45, 0x46, 0xE7, 0x52, 0xA9, 0xB5, 0xA1, 0x2B, 0xA7, 0xDA, 0x15, 0x66, 0x8E, 0x7C, 0x8A, 0xF2, 
0x84, 0xD0, 0xBB, 0x3C, 0x32, 0x0E, 0x6E, 0xA5, 0xDF, 0x92, 0x19, 0x5C, 0xB2, 0x62, 0x16, 0xF1, 
0x55, 0x75, 0x4F, 0x77, 0x8F, 0x24, 0xBA, 0x41, 0xF1, 0xE7, 0x95, 0xE4, 0x04, 0xB3, 0x61, 0x44, 
0xB5, 0xAF, 0x80, 0x8E, 0xC3, 0xEE, 0xBB, 0x98, 0x20, 0xDB, 0x00, 0x7E, 0xF9, 0xE1, 0x03, 0xE4, 
0x2F, 0x7B, 0x3F, 0x76, 0xDE, 0x89, 0x67, 0xA0, 0x48, 0x83, 0x56, 0x07, 0x36, 0xD3, 0x73, 0xE5, 
0x6C, 0xCA, 0x92, 0x5B, 0x52, 0x92, 0xD9, 0x29, 0xA4, 0x8A, 0x62, 0x45, 0x42, 0x1A, 0x1B, 0x3B, 
0x5F, 0x4E, 0x51, 0xFE, 0x58, 0xBC, 0xCE, 0x78, 0xE2, 0xAA, 0xCA, 0x48, 0x3E, 0x79, 0xA8, 0xFE, 
0x13, 0xC3, 0x89, 0x44, 0x7A, 0x0C, 0x9E, 0xBE, 0x5C, 0xE3, 0xF3, 0x60, 0x92, 0x87, 0xF4, 0x04, 
0xC8, 0xDD, 0xBA, 0x84, 0x3B, 0x5F, 0x19, 0x11, 0x73, 0x8B, 0x25, 0xD5, 0xC6, 0x86, 0x4E, 0x97, 
0xF1, 0xE6, 0xFF, 0xBC, 0x46, 0x45, 0x42, 0x5A, 0xE3, 0x02, 0x49, 0x49, 0xCE, 0xF6, 0xAF, 0xB8, 
0x66, 0xEE, 0xE3, 0x64, 0xEF, 0x5A, 0x61, 0x8E, 0xB8, 0x06, 0xA2, 0xC4, 0x39, 0x2B, 0x93, 0x07, 
0xFE, 0x3C, 0x3B, 0xE1, 0xBD, 0x39, 0x07, 0x83, 0x4A, 0x7F, 0xE6, 0x3F, 0x0C, 0x64, 0x46, 0x30, 
0x27, 0x0D, 0x2E, 0x98, 0xFC, 0xA3, 0x29, 0x7C, 0x80, 0x2E, 0x9B, 0xFF, 0xC3, 0xA4, 0xC9, 0x3F, 
0x23, 0xFD, 0xC8, 0xAE, 0xC6, 0x05, 0x12, 0x02, 0xBB, 0xE6, 0xA5, 0xCD, 0xBE, 0xF5, 0xA6, 0xE8, 
0x0A, 0x05, 0x47, 0x46, 0xFA, 0x6B, 0xC1, 0x16, 0x3E, 0x9C, 0x65, 0x00, 0x21, 0x94, 0x7A, 0x77, 
0x98, 0xC8, 0xDD, 0xBA, 0x24, 0x26, 0x6A, 0xC1, 0x0D, 0xA5, 0x59, 0x59, 0x09, 0xD7, 0xCD, 0xFE, 
0x43, 0xC0, 0x4F, 0xCC, 0xA0, 0x90, 0x92, 0x8B, 0x57, 0xCC, 0x62, 0xC2, 0x8C, 0xB1, 0x38, 0x82, 
0x58, 0xC4, 0x2A, 0x1C, 0xA8, 0x89, 0xF1, 0x95, 0xEE, 0x5E, 0x3D, 0x9E, 0x69, 0xC8, 0x39, 0x0D, 
0x6E, 0x2A, 0x1B, 0xF4, 0xF5, 0xF4, 0xCD, 0x3F, 0xA6, 0xBB, 0xFF, 0x4C, 0xBA, 0x6E, 0x5C, 0x48, 
0xEF, 0x6F, 0xDE, 0x6B, 0xF0, 0x79, 0xA9, 0x47, 0x77, 0x33, 0xE6, 0x5F, 0x77, 0x33, 0xEE, 0x9D, 
0x5B, 0x2D, 0x6D, 0x35, 0x14, 0x0E, 0x71, 0x7F, 0xC2, 0xF4, 0xE0, 0xEF, 0x7E, 0x68, 0xE0, 0xC4, 
0x10, 0x89, 0xFC, 0x22, 0xBF, 0x85, 0xF7, 0x7F, 0xCB, 0xED, 0x4A, 0xA1, 0xF4, 0x19, 0x31, 0xBF, 
0x91, 0x44, 0x20, 0x86, 0x16, 0x3C, 0xC3, 0x9A, 0x6E, 0xC3, 0x6C, 0x55, 0xEE, 0xA4, 0xFA, 0xFE, 
0x3E, 0xBD, 0xE5, 0x81, 0xCD, 0xDA, 0xCF, 0x16, 0xCF, 0x20, 0xFF, 0x9B, 0x77, 0xB1, 0xFB, 0xAD, 
0xB9, 0xEB, 0x4F, 0xA3, 0x36, 0x4B, 0x2C, 0xCE, 0xFC, 0xF5, 0xE8, 0xD7, 0x79, 0xAC, 0x61, 0xAB, 
0x99, 0x54, 0x6F, 0x08, 0x9A, 0x76, 0x74, 0x96, 0x10, 0xB5, 0xEF, 0x10, 0x66, 0x08, 0xE5, 0xB2, 
0x79, 0x37, 0x37, 0x0B, 0xAA, 0x76, 0x19, 0xCB, 0x0C, 0x9D, 0x51, 0xDA, 0x57, 0x18, 0xC6, 0x23, 
0x08, 0x51, 0x6D, 0x72, 0xA8, 0x84, 0x32, 0x81, 0xF8, 0x02, 0x21, 0xBF, 0xD0, 0x34, 0xED, 0x93, 
0x05, 0x13, 0x32, 0xA2, 0x6C, 0xA3, 0xA0, 0x86, 0x13, 0xF4, 0x13, 0x40, 0xC2, 0xEE, 0x79, 0x37, 
0x37, 0xFB, 0x22, 0x8C, 0x5A, 0xA2, 0x86, 0xB9, 0x37, 0xA7, 0x2E, 0x01, 0xAE, 0xB0, 0x5A, 0x47, 
0x24, 0x08, 0xBE, 0x0E, 0x20, 0x79, 0x8C, 0x7A, 0x9A, 0x15, 0x9B, 0x88, 0x3D, 0x82, 0x32, 0x80, 
0x94, 0x94, 0xF9, 0x85, 0xED, 0xC7, 0xB1, 0x56, 0x4A, 0x13, 0xD5, 0x38, 0xDB, 0x00, 0xB5, 0x4D, 
0xC0, 0x98, 0x5D, 0x30, 0x3E, 0x39, 0xC2, 0xBB, 0x20, 0x37, 0x11, 0x09, 0xAA, 0xB7, 0x03, 0x48, 
0x19, 0x68, 0x3C, 0xB4, 0x21, 0xA4, 0xED, 0xD5, 0x08, 0xE9, 0x69, 0x22, 0xC2, 0xD4, 0x5B, 0x09, 
0x94, 0x52, 0xAE, 0x9E, 0xFB, 0x9B, 0xE4, 0x86, 0xAD, 0xD3, 0xDE, 0x44, 0xCC, 0x50, 0x7F, 0x1D, 
0x40, 0x51, 0x7E, 0x5C, 0x83, 0xE4, 0x9B, 0xA8, 0xC6, 0x59, 0x7D, 0x01, 0x9C, 0xBD, 0x17, 0x4A, 
0x25, 0x9A, 0xED, 0x8D, 0x08, 0xEA, 0x69, 0x22, 0xC2, 0x9C, 0x55, 0x07, 0x60, 0xDF, 0x59, 0xC7, 
0x0B, 0xE7, 0x4E, 0x48, 0x8A, 0xFD, 0xB1, 0xD1, 0x4D, 0xD4, 0x4A, 0x8D, 0x57, 0x80, 0xAC, 0xF6, 
0xAD, 0x6F, 0xBC, 0x1D, 0x49, 0x31, 0x4D, 0x44, 0x9E, 0xBA, 0x46, 0x05, 0x97, 0x6B, 0x95, 0xBE, 
0xEC, 0x05, 0x77, 0xB4, 0x0C, 0x69, 0xA6, 0x64, 0x71, 0x3A, 0xC3, 0x0C, 0x0F, 0x3D, 0x1A, 0xAD, 
0xB0, 0x09, 0xEC, 0xF1, 0x7C, 0x90, 0x79, 0x30, 0x3C, 0x6B, 0x24, 0xD4, 0xFA, 0x15, 0x20, 0x25, 
0x45, 0xA1, 0x16, 0x3E, 0x40, 0xD1, 0x11, 0x8A, 0xBA, 0xC7, 0xF1, 0x07, 0xA7, 0xC2, 0x39, 0xB3, 
0xDE, 0x8E, 0xE9, 0x48, 0x64, 0xB9, 0xC1, 0x2B, 0x17, 0x1C, 0x24, 0x6C, 0x4B, 0xB8, 0x07, 0xFA, 
0x0A, 0xF8, 0xF2, 0x54, 0xE6, 0x8D, 0x7B, 0xFC, 0x8F, 0x82, 0xD2, 0x9D, 0x95, 0x5C, 0xE5, 0x37, 
0x28, 0x68, 0x4C, 0x9C, 0x73, 0x16, 0x89, 0xEE, 0xD1, 0x79, 0xEA, 0xFD, 0x2A, 0xEE, 0xA0, 0x8E, 
0x89, 0x1D, 0x8D, 0xA5, 0xA6, 0x01, 0x0C, 0xB9, 0x07, 0x38, 0x89, 0x74, 0x36, 0xBA, 0xE0, 0x06, 
0x41, 0xD9, 0xEE, 0x4A, 0xAE, 0xA9, 0x32, 0x78, 0x1D, 0xD9, 0xD4, 0x8F, 0x10, 0x2C, 0x86, 0xC4, 
0x7B, 0x5C, 0xE3, 0x77, 0xB9, 0x1E, 0x1E, 0x7C, 0x2C, 0x8C, 0x85, 0x0F, 0x81, 0xE6, 0x06, 0xAA, 
0x62, 0x0B, 0xB0, 0xC1, 0xAC, 0xDA, 0xFF, 0x00, 0xF0, 0xAC, 0xA9, 0xE4, 0x37, 0xE5, 0x3A, 0x0F, 
0x49, 0x89, 0xB5, 0x9D, 0xE6, 0x31, 0x80, 0x2E, 0x39, 0x74, 0x42, 0x63, 0x64, 0x57, 0x2F, 0xAF, 
0x12, 0x81, 0xCE, 0xB7, 0x1A, 0x75, 0x00, 0xCD, 0xA0, 0x40, 0x41, 0x1E, 0x33, 0x33, 0x93, 0x51, 
0xA0, 0xE3, 0xE1, 0xC9, 0xF5, 0x4E, 0xD6, 0x26, 0xDA, 0x98, 0xA1, 0x0A, 0x82, 0x1A, 0x7B, 0x78, 
0xAE, 0xE1, 0x37, 0x58, 0x79, 0xA2, 0x92, 0x31, 0x79, 0x35, 0xDB, 0x63, 0xC2, 0x46, 0xC4, 0x47, 
0x75, 0x2F, 0x83, 0xEC, 0x34, 0x37, 0xD3, 0x1D, 0x0A, 0xFD, 0x11, 0x31, 0x39, 0xAA, 0xDC, 0x7C, 
0x24, 0x5A, 0xA5, 0xC1, 0xD4, 0xCD, 0x55, 0x3C, 0x70, 0x35, 0x9C, 0x8C, 0x64, 0xD6, 0x96, 0x14, 
0xC0, 0x4C, 0x70, 0x5C, 0xE4, 0xE2, 0xBE, 0x38, 0x95, 0x49, 0x42, 0x10, 0xDA, 0x3C, 0xAD, 0x1F, 
0x09, 0xBA, 0x64, 0xFF, 0x49, 0x3F, 0xB7, 0x77, 0xF5, 0xD5, 0x3D, 0x8D, 0x3B, 0x5C, 0x58, 0x7A, 
0x07, 0xAE, 0xB2, 0xD3, 0x2D, 0xC5, 0xCE, 0x6B, 0x76, 0x41, 0xAF, 0x73, 0xEE, 0x69, 0x20, 0xD1, 
0xBD, 0x06, 0xFF, 0xDC, 0x5F, 0xC5, 0x3D, 0xFD, 0xC0, 0xB2, 0x55, 0xA9, 0x2C, 0xBF, 0xE8, 0x53, 
0xC0, 0xDE, 0xCF, 0xC9, 0x04, 0xB7, 0x8D, 0x47, 0xD4, 0x53, 0x4B, 0xD3, 0x59, 0xAE, 0x29, 0xAC, 
0x48, 0xA4, 0x2E, 0xD9, 0x70, 0x54, 0xE7, 0xDE, 0x1E, 0x5E, 0x3E, 0xB5, 0x5A, 0x4E, 0xD4, 0x5C, 
0xEC, 0xF9, 0xD0, 0xBC, 0x83, 0x9B, 0x47, 0x1C, 0x0A, 0xE3, 0x85, 0xC0, 0x65, 0xB5, 0x1E, 0xD3, 
0x91, 0x48, 0x1D, 0x0E, 0x7B, 0x34, 0x9E, 0x5C, 0xE7, 0xE5, 0xD5, 0x51, 0xE0, 0xB3, 0x5A, 0x12, 
0x44, 0x91, 0x01, 0x4E, 0xB3, 0x0C, 0xB2, 0xD3, 0xE2, 0x78, 0xC8, 0x2E, 0xB8, 0x5E, 0x08, 0x02, 
0x6E, 0x77, 0x1A, 0x6B, 0x18, 0x92, 0x52, 0xAF, 0xCE, 0x0B, 0xBB, 0x3D, 0xBC, 0x38, 0x08, 0x6A, 
0x6E, 0xDD, 0x65, 0x21, 0x51, 0x67, 0x80, 0xD3, 0xAC, 0x80, 0xF3, 0x53, 0xDC, 0xDC, 0x65, 0x57, 
0xB8, 0x41, 0x11, 0x24, 0x59, 0xAD, 0x27, 0x14, 0x34, 0x83, 0x3D, 0x7E, 0xC9, 0xCB, 0xC5, 0x55, 
0x4C, 0x1B, 0x00, 0xD1, 0x33, 0x7D, 0xF8, 0x0C, 0xA2, 0xD6, 0x00, 0xA7, 0x29, 0x80, 0x16, 0xED, 
0xDC, 0x8C, 0x75, 0x0A, 0xC6, 0xA9, 0x82, 0xCE, 0x51, 0x5F, 0x59, 0x94, 0x68, 0x3E, 0x83, 0x42, 
0x9F, 0xCE, 0xF4, 0x05, 0x3E, 0x3E, 0xFA, 0x3D, 0x54, 0x59, 0x2D, 0xA9, 0x2E, 0xA2, 0xFB, 0x62, 
0x9E, 0xC1, 0xE7, 0x60, 0x4B, 0xB6, 0x71, 0x69, 0x92, 0x8D, 0xD1, 0x76, 0x85, 0x61, 0x8A, 0xA0, 
0x55, 0xD4, 0x98, 0x41, 0x62, 0xE8, 0x92, 0x8D, 0x5E, 0x98, 0x5D, 0xAA, 0xF1, 0x7E, 0xBE, 0x8F, 
0xCD, 0xC4, 0xC8, 0x10, 0xFA, 0xE8, 0xB8, 0x80, 0x0D, 0x64, 0x0E, 0xC4, 0xB5, 0x75, 0x91, 0x1F, 
0x27, 0x18, 0xAC, 0x0A, 0xAE, 0xB0, 0x09, 0x3A, 0x0A, 0x81, 0x79, 0xAB, 0x45, 0x06, 0x81, 0x61, 
0x50, 0xA1, 0xC1, 0x6A, 0xCD, 0x60, 0x41, 0xA5, 0xCE, 0xFC, 0xE3, 0x7E, 0xBE, 0x1D, 0x00, 0xD1, 
0xB9, 0x5C, 0x48, 0x1D, 0xC4, 0xA4, 0x01, 0xCE, 0xE4, 0x3A, 0x50, 0x7F, 0x0F, 0x59, 0xC9, 0x0E, 
0x7A, 0x39, 0x54, 0xFA, 0xA8, 0xD0, 0x53, 0x28, 0xE4, 0xA8, 0x90, 0x8A, 0x30, 0x67, 0xBB, 0x19, 
0x29, 0xF1, 0x49, 0x38, 0xE4, 0x93, 0x6C, 0x92, 0x06, 0xDF, 0xF8, 0x24, 0x4B, 0xCA, 0xBC, 0xAC, 
0xEE, 0xC3, 0xD9, 0xDB, 0x30, 0xC6, 0x1E, 0x31, 0x6F, 0x80, 0x40, 0xBC, 0x05, 0xF1, 0x1D, 0x9D, 
0xB4, 0x4E, 0xD4, 0xC8, 0xC1, 0x49, 0x7B, 0xBB, 0x4E, 0x7B, 0xD5, 0x46, 0xA6, 0x5F, 0x92, 0x66, 
0x97, 0xA4, 0x2B, 0x92, 0x78, 0xA1, 0xE0, 0x14, 0x02, 0x27, 0x12, 0x03, 0x89, 0xD7, 0x90, 0x78, 
0x0D, 0xC1, 0x09, 0x4D, 0x70, 0xC4, 0x09, 0x47, 0xBD, 0x06, 0xFB, 0x04, 0xEC, 0xAC, 0xD0, 0xD8, 
0xA9, 0x69, 0x6C, 0x5F, 0x09, 0x25, 0x13, 0xE0, 0x9C, 0xDA, 0x4B, 0xA0, 0x89, 0x26, 0x9A, 0x68, 
0xA2, 0x89, 0x26, 0x9A, 0xF8, 0xF1, 0xF2, 0xFF, 0xFE, 0x3C, 0x31, 0x41, 0x54, 0xB9, 0xDD, 0x19, 
0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};

const wxBitmap& GetLogo()
{
    static const wxBitmap s_logo(wxBITMAP_PNG_FROM_DATA(webpier_logo));
    return s_logo;
}

class CTaskBarIcon : public wxTaskBarIcon
{
    CMainFrame* m_frame;

public:

    enum
    {
        PU_CONFIGURE = 10001,
        PU_START,
        PU_STOP,
        PU_QUIT
    };

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    CTaskBarIcon(wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE)
    :   wxTaskBarIcon(iconType)
#else
    CTaskBarIcon()
#endif
    {
        m_frame = new CMainFrame();
        m_frame->Show(false);

        wxIcon icon;
        icon.CopyFromBitmap(::GetLogo());
        m_frame->SetIcon(icon);
        if (!this->SetIcon(icon))
        {
            wxLogError("Could not set icon.");
        }
    }

    ~CTaskBarIcon() override
    {
        delete m_frame;
    }

    void OnLeftButtonDClick(wxTaskBarIconEvent&)
    {
        m_frame->Show(true);
    }

    void OnMenuConfigure(wxCommandEvent&)
    {
        m_frame->Show(true);
    }

    void OnMenuStart(wxCommandEvent&)
    {

    }

    void OnMenuStop(wxCommandEvent&)
    {

    }

    void OnMenuExit(wxCommandEvent&)
    {
        m_frame->Close(true);
        this->Destroy();
    }

    virtual wxMenu* CreatePopupMenu() wxOVERRIDE
    {
        wxMenu *menu = new wxMenu();
        menu->Append(PU_CONFIGURE, _("&Configure..."));
        wxMenu* submenu = new wxMenu();
        submenu->Append(PU_START, _("&Start"));
        submenu->Append(PU_STOP, _("S&top"));
        menu->Append(wxID_ANY, "&Daemon", submenu);

    #ifdef __WXOSX__
        if ( OSXIsStatusItem() )
    #endif
        menu->Append(PU_QUIT, _("&Quit"));

        return menu;
    }

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(CTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(PU_CONFIGURE, CTaskBarIcon::OnMenuConfigure)
    EVT_MENU(PU_START, CTaskBarIcon::OnMenuStart)
    EVT_MENU(PU_STOP, CTaskBarIcon::OnMenuStop)
    EVT_MENU(PU_QUIT, CTaskBarIcon::OnMenuExit)
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLeftButtonDClick)
wxEND_EVENT_TABLE()

class CWebPierApp : public wxApp
{
public:
    bool OnInit() override
    {
        if (!wxApp::OnInit() )
            return false;

        wxImage::AddHandler(new wxPNGHandler);

        if (!wxTaskBarIcon::IsAvailable())
        {
            wxMessageBox(
                "There appears to be no system tray support in your current environment. This sample may not behave as expected.",
                "Warning",
                wxOK | wxICON_EXCLAMATION);
        }

        CTaskBarIcon* icon = new CTaskBarIcon();
        (void)icon;

        return true;
    }
};

wxIMPLEMENT_APP(CWebPierApp);

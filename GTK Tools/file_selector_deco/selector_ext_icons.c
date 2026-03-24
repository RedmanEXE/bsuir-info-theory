//
// Created by REXE on 23.03.26.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "selector_ext_icons.h"

typedef struct
{
    const char *ext;
    const char *icon;
} FileSelector_ExtensionEntry;

static const FileSelector_ExtensionEntry ext_table[] = {
    {".7z",     "package-x-generic-symbolic"},
    {".apk",    "application-x-sharedlib-symbolic"},
    {".app",    "application-x-sharedlib-symbolic"},
    {".bin",    "application-x-firmware-symbolic"},
    {".bmp",    "image-x-generic-symbolic"},
    {".cer",    "application-certificate-symbolic"},
    {".crt",    "application-certificate-symbolic"},
    {".deb",    "application-x-sharedlib-symbolic"},
    {".der",    "application-certificate-symbolic"},
    {".doc",    "x-office-document-symbolic"},
    {".docx",   "x-office-document-symbolic"},
    {".elf",    "application-x-sharedlib-symbolic"},
    {".exe",    "application-x-sharedlib-symbolic"},
    {".flac",   "audio-x-generic-symbolic"},
    {".fodg",   "x-office-drawing-symbolic"},
    {".fodp",   "x-office-presentation-symbolic"},
    {".fods",   "x-office-spreadsheet-symbolic"},
    {".fodt",   "x-office-document-symbolic"},
    {".gif",    "image-x-generic-symbolic"},
    {".ico",    "image-x-generic-symbolic"},
    {".ics",    "x-office-calendar-symbolic"},
    {".ipa",    "application-x-sharedlib-symbolic"},
    {".jpeg",   "image-x-generic-symbolic"},
    {".jpg",    "image-x-generic-symbolic"},
    {".mp3",    "audio-x-generic-symbolic"},
    {".mp4",    "video-x-generic-symbolic"},
    {".mpg",    "audio-x-generic-symbolic"},
    {".odg",    "x-office-drawing-symbolic"},
    {".odp",    "x-office-presentation-symbolic"},
    {".ods",    "x-office-spreadsheet-symbolic"},
    {".odt",    "x-office-document-symbolic"},
    {".ogg",    "audio-x-generic-symbolic"},
    {".otf",    "font-x-generic-symbolic"},
    {".pdf",    "x-office-document-symbolic"},
    {".pem",    "application-certificate-symbolic"},
    {".pfx",    "application-certificate-symbolic"},
    {".png",    "image-x-generic-symbolic"},
    {".ppt",    "x-office-presentation-symbolic"},
    {".pptx",   "x-office-presentation-symbolic"},
    {".psd",    "image-x-generic-symbolic"},
    {".rar",    "package-x-generic-symbolic"},
    {".rpm",    "application-x-sharedlib-symbolic"},
    {".svg",    "image-x-generic-symbolic"},
    {".ttf",    "font-x-generic-symbolic"},
    {".txt",    "text-x-generic-symbolic"},
    {".vcf",    "x-office-address-book-symbolic"},
    {".vsd",    "x-office-drawing-symbolic"},
    {".vsdx",   "x-office-drawing-symbolic"},
    {".wav",    "audio-x-generic-symbolic"},
    {".xls",    "x-office-spreadsheet-symbolic"},
    {".xlsx",   "x-office-spreadsheet-symbolic"},
    {".zip",    "package-x-generic-symbolic"},
};

static int FileSelector_CompareExtensions(const void *a, const void *b)
{
    return strcmp(((const FileSelector_ExtensionEntry *)a)->ext, ((const FileSelector_ExtensionEntry *)b)->ext);
}

const char *FileSelector_GetIconByExtension(const char *ext)
{
    if (!ext)
        return "dialog-question";

    FileSelector_ExtensionEntry key = { ext, NULL };
    size_t table_size = sizeof(ext_table) / sizeof(ext_table[0]);

    FileSelector_ExtensionEntry *result = bsearch(&key, ext_table, table_size, sizeof(FileSelector_ExtensionEntry),
                                                  FileSelector_CompareExtensions);

    return result ? result->icon : "dialog-question";
}
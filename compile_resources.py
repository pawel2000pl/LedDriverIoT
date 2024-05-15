import os
import re
import ctypes
import mimetypes
import json

os.system('gcc --std=c11 -O3 -x c main/fastlz.cpp -fpic -shared -o fastlz.so')

lib = ctypes.CDLL('./fastlz.so')
fastlz_compress_level = lib.fastlz_compress_level
fastlz_compress_level.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p]
fastlz_compress_level.restype = ctypes.c_int

fastlz_decompress = lib.fastlz_decompress
fastlz_decompress.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_int]
fastlz_decompress.restype = ctypes.c_int

def compress_data(data: bytes, level: int = 2) -> bytes:
    # Sprawdzenie długości danych
    if len(data) < 16:
        raise ValueError("Input buffer size must be at least 16 bytes.")
    
    # Utworzenie bufora wynikowego - minimalna wielkość 5% większa niż bufor wejściowy
    output_buffer_size = int(len(data) * 1.05)
    if output_buffer_size < 66:
        output_buffer_size = 66
    output_buffer = ctypes.create_string_buffer(output_buffer_size)
    
    # Kompresja danych
    result_size = fastlz_compress_level(level, data, len(data), output_buffer)
    
    # Sprawdzenie czy kompresja się powiodła
    if result_size <= 0:
        raise ValueError("Compression failed.")
    
    # Zwrócenie skompresowanych danych jako bytes
    return ctypes.string_at(output_buffer, result_size)



def decompress_data(compressed_data: bytes, max_decompressed_size: int = 65536) -> bytes:
    # Utworzenie bufora dla danych zdekompresowanych
    output_buffer = ctypes.create_string_buffer(max_decompressed_size)

    # Dekompresja danych
    decompressed_size = fastlz_decompress(compressed_data, len(compressed_data), output_buffer, max_decompressed_size)

    # Sprawdzenie czy dekompresja się powiodła
    if decompressed_size == 0:
        raise ValueError("Dekompresja nie powiodła się.")
    
    # Zwrócenie zdekompresowanych danych jako bytes
    return ctypes.string_at(output_buffer, decompressed_size)


def str2c(c_str):
    if isinstance(c_str, bytes):
        return ", ".join([str(c) for c in c_str])
    c_str = str(c_str)
    return ", ".join([str(ord(c)) for c in c_str])


info_struct = """
#pragma once

struct Resource {
    const char* name;
    const char* mime_type;
    const unsigned char* data;
    const unsigned int size;
    const unsigned int decompressed_size;
};

"""


PATH = 'resources/'
result_header = [info_struct]
result_content = ['#include "resources.h"', '']
resource_names = []
max_decompressed_size = 0
max_compressed_size = 0
total_compressed = 0

operators = ['<', '>', '<=', '>=', '=', '==', '===', '\\+', '-', '\\*', ',', ':', '\\/', '\\[', '\\]', '\\/=', '\\*=', '\\+=', '-=', '\\(', '\\)', '\\{', '\\}', ';', '\\|', '\\^', '&']

for root, dirs, files in os.walk(PATH, topdown=False):
    for filename in files:
        with open(PATH+filename) as f: content = f.read()
        orginal_content = content
        if filename.endswith('.json'):
            content = json.dumps(json.loads(content), indent=None)
        else:
            content = re.sub('/\\*(.|\n)*?\\*/', ' ', content)
            content = re.sub('\\/\\/[^"\'\\n]*\n', ' ', content)
            content = re.sub('^[\\s]*', ' ', content)
            content = re.sub('[\\s]*$', ' ', content)
            content = re.sub('[\\s]+', ' ', content)
            # for operator in operators:
            #     no_escaped = operator if operator[0] != '\\' else operator[1:]
            #     content = re.sub('[\\s]*'+operator+'[\\s]*', no_escaped, content)
        compressed = compress_data(content.encode('utf-8'))
        decompressed = decompress_data(compressed).decode('utf-8')
        assert content == decompressed        
        var_name = filename.replace('.', '_')
        resource_names.append('resource_'+var_name)

        result_header.append('extern const unsigned char '+var_name+'_data[];')
        result_header.append('extern const char '+var_name+'_filename[];')
        result_header.append('extern const char '+var_name+'_mime_type[];')
        result_header.append('extern const unsigned int '+var_name+'_size;')
        result_header.append('extern const unsigned int '+var_name+'_decompressed_size;')
        result_header.append('extern const struct Resource resource_'+var_name+';')
        result_header.append('')

        result_content.append('const unsigned char '+var_name+'_data[] = {'+str2c(compressed)+'};')
        result_content.append('const char '+var_name+'_filename[] = {'+str2c('/'+filename+"\0")+'};')
        result_content.append('const char '+var_name+'_mime_type[] = {'+str2c(mimetypes.guess_type(PATH+filename)[0]+"\0")+'};')
        result_content.append('const unsigned int '+var_name+'_size = '+str(len(compressed))+';')
        result_content.append('const unsigned int '+var_name+'_decompressed_size = '+str(len(content))+';')
        result_content.append('const struct Resource resource_'+var_name+' = {')
        result_content.append(' .name = '+var_name+'_filename,')
        result_content.append(' .mime_type = '+var_name+'_mime_type,')
        result_content.append(' .data = '+var_name+'_data,')
        result_content.append(' .size = '+var_name+'_size,')
        result_content.append(' .decompressed_size = '+var_name+'_decompressed_size')
        result_content.append('};')
        result_content.append('')

        max_decompressed_size = max(max_decompressed_size, len(content))
        max_compressed_size = max(max_compressed_size, len(compressed))
        total_compressed += len(compressed)
        print(filename, len(orginal_content), len(content), len(compressed))


print("Max decompressed:", max_decompressed_size)
print("Max compressed:", max_compressed_size)
print("Total compressed:", total_compressed)


result_header.append('extern const struct Resource* resources[];')
result_header.append('extern const unsigned int resources_count;')
result_header.append('')
result_header.append('extern const unsigned int max_resource_compressed_buffer;')
result_header.append('extern const unsigned int max_resource_decompressed_buffer;')
result_header.append('')

result_content.append('const struct Resource* resources[] = {'+','.join(['&'+name for name in resource_names])+'};')
result_content.append('const unsigned int resources_count = '+str(len(resource_names))+';')
result_content.append('')
result_content.append('const unsigned int max_resource_compressed_buffer = '+str(max_compressed_size)+';')
result_content.append('const unsigned int max_resource_decompressed_buffer = '+str(max_decompressed_size)+';')
result_content.append('')

with open('main/resources.h', 'w') as f: f.write("\n".join(result_header))
with open('main/resources.cpp', 'w') as f: f.write("\n".join(result_content))

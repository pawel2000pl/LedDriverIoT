#!/usr/bin/python3

import os
import re
import json
import base64
import hashlib
import mimetypes

from typing import Union


def filehash(filename):
    sha1 = hashlib.sha1()
    sha1.update(open(filename, 'rb').read())
    return base64.b64encode(sha1.digest()).decode('utf-8')


def str2c(c_str):
    if isinstance(c_str, bytes):
        return ", ".join([str(c) for c in c_str])
    c_str = str(c_str)
    return ", ".join([str(ord(c)) for c in c_str])


def find_best_divide_letter(name_list):
    name_list_len = len(name_list)
    best_letter = 'k'
    best_pos = 0
    best_value = 2*name_list_len

    for i in range(min(len(name) for name in name_list)):
        letters = list(set(name[i] for name in name_list))
        for letter in letters:
            part_count = sum(1 for name in name_list if ord(name[i]) <= ord(letter))
            if abs(name_list_len - 2*part_count) < best_value:
                best_value = abs(name_list_len - 2*part_count)
                best_letter = letter
                best_pos = i

    return best_pos, best_letter


def create_name_resolver(name_list, indent=2):
    indent_s = " " * indent
    if len(name_list) == 1:
        res_str = 'resource_' + name_list[0].replace('.', '_').replace('/', '')
        return indent_s + 'return (def && (!same_str(name, '+res_str+'.name))) ? *def : ' + res_str + ';'

    pos, letter = find_best_divide_letter([name + '\0' for name in name_list])
    list1 = [name for name in name_list if len(name) == pos or ord(name[pos]) <= ord(letter)]
    list2 = [name for name in name_list if len(name) > pos and ord(name[pos]) > ord(letter)]
    result = [
        indent_s, "if (name[%d] <= %d)"%(pos, ord(letter)), " {\n",
        create_name_resolver(list1, indent+2), "\n",
        indent_s, "}\n",
        create_name_resolver(list2, indent),
    ]
    return str().join(result)


info_struct = """
#pragma once

struct Resource {
    const char* name;
    const char* mime_type;
    const char* etag;
    const unsigned char* data;
    const unsigned int size;
};

"""

same_str = """
int same_str(const char* a, const char* b) {
    while (*a || *b)
        if (*(a++) != *(b++)) return 0;
    return 1;
}
"""

PATH = 'resources/'
result_header = [info_struct]
result_content = ['#include "resources.h"', '']
resource_names = []
all_srcs = dict()
max_non_minified_size = 0
max_minified_size = 0
total_non_minified = 0
total_minified = 0

operators = ['<', '>', '<=', '>=', '=', '==', '===', '\\+', '-', '\\*', ',', ':', '\\/', '\\[', '\\]', '\\/=', '\\*=', '\\+=', '-=', '\\(', '\\)', '\\{', '\\}', ';', '\\|', '\\^', '&']
minify_extensions = {'html', 'js', 'json', 'svg', 'css'}

for root, dirs, files in os.walk(PATH, topdown=False):
    for filename in files:
        content: Union[bytes,str]
        with open(PATH+filename, 'br') as f: content = f.read()
        orginal_content = content
        all_srcs[PATH+filename] = content
        if filename.split('.')[-1] in minify_extensions:
            content = content.decode('utf-8')
            if filename.endswith('.json'):
                content = json.dumps(json.loads(content), indent=None, separators=(',', ':'))
            else:
                content = re.sub('/\\*(.|\n)*?\\*/', ' ', content)
                content = re.sub('\\/\\/[^"\'\\n]*\n', ' ', content)
                content = re.sub('^[\\s]*', '', content)
                content = re.sub('[\\s]*$', '', content)
                content = re.sub('[\\s]+', ' ', content)
                # for operator in operators:
                #     no_escaped = operator if operator[0] != '\\' else operator[1:]
                #     content = re.sub('[\\s]*'+operator+'[\\s]*', no_escaped, content)
            while len(content) < 16: content += " "
            content = content.encode('utf-8')

        while len(content) < 16: content += b"\0"
        var_name = filename.replace('.', '_')
        resource_names.append('resource_'+var_name)
        etag = '"' + filehash(PATH+filename) + '"'

        result_header.append('extern const unsigned char '+var_name+'_data[];')
        result_header.append('extern const char '+var_name+'_filename[];')
        result_header.append('extern const char '+var_name+'_mime_type[];')
        result_header.append('extern const char '+var_name+'_etag[];')
        result_header.append('extern const unsigned int '+var_name+'_size;')
        result_header.append('extern const struct Resource resource_'+var_name+';')
        result_header.append('')

        result_content.append('const unsigned char '+var_name+'_data[] = {'+str2c(content+b"\0")+'};')
        result_content.append('const char '+var_name+'_filename[] = {'+str2c('/'+filename+"\0")+'};')
        result_content.append('const char '+var_name+'_mime_type[] = {'+str2c(str(mimetypes.guess_type(PATH+filename)[0])+"\0")+'};')
        result_content.append('const char '+var_name+'_etag[] = {'+str2c(etag+"\0")+'};')
        result_content.append('const unsigned int '+var_name+'_size = '+str(len(content))+';')
        result_content.append('const struct Resource resource_'+var_name+' = {')
        result_content.append(' .name = '+var_name+'_filename,')
        result_content.append(' .mime_type = '+var_name+'_mime_type,')
        result_content.append(' .etag = '+var_name+'_etag,')
        result_content.append(' .data = '+var_name+'_data,')
        result_content.append(' .size = '+var_name+'_size,')
        result_content.append('};')
        result_content.append('')

        max_non_minified_size = max(max_non_minified_size, len(orginal_content))
        max_minified_size = max(max_minified_size, len(content))
        total_non_minified += len(orginal_content)
        total_minified += len(content)
        print(filename, len(orginal_content), len(content))


print("Max non-minified:", max_non_minified_size)
print("Max minified:", max_minified_size)
print("Total before compression:", total_non_minified)
print("Total minified:", total_minified)
print("Compression rate: %.4f"%(total_minified/total_non_minified))

all_srcs_keys = list(all_srcs)
all_srcs_keys.sort()
whole_src = b"\n\n".join([all_srcs[k] for k in all_srcs_keys])
sha1 = hashlib.sha1()
sha1.update(whole_src)

result_header.append('extern const struct Resource* resources[];')
result_header.append('extern const unsigned int resources_count;')
result_header.append('')
result_header.append('extern const unsigned int max_resource_minified_buffer;')
result_header.append('extern const unsigned int max_resource_non_minified_buffer;')
result_header.append('')
result_header.append('extern const char* RESOURCES_SHA1;')
result_header.append('')
result_header.append('const struct Resource& getResourceByName(const char* name, const struct Resource* def=0);')
result_header.append('')

result_content.append('const struct Resource* resources[] = {'+','.join(['&'+name for name in resource_names])+'};')
result_content.append('const unsigned int resources_count = '+str(len(resource_names))+';')
result_content.append('')
result_content.append('const unsigned int max_resource_minified_buffer = '+str(max_minified_size)+';')
result_content.append('const unsigned int max_resource_non_minified_buffer = '+str(max_non_minified_size)+';')
result_content.append('')
result_content.append(same_str)
result_content.append('')
result_content.append('const char* RESOURCES_SHA1 = "'+sha1.hexdigest()+'";')
result_content.append('')
result_content.append('const struct Resource& getResourceByName(const char* name, const struct Resource* def) {')
result_content.append(create_name_resolver([rs[len('resources'):] for rs in all_srcs.keys()]))
result_content.append('}')
result_content.append('')

with open('main/src/resources.h', 'w') as fw: fw.write("\n".join(result_header))
with open('main/src/resources.cpp', 'w') as fw: fw.write("\n".join(result_content))

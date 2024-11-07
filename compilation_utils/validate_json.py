import re
import json
from sys import argv

# Translated from main/validate_json.cpp with ChatGPT

# Funkcja sprawdzająca zakres liczbowy
def validate_range(value, schema):
    if "min_value" in schema and value < schema["min_value"]:
        return False
    if "max_value" in schema and value > schema["max_value"]:
        return False
    return True


# Lista typów prostych JSON
json_simple_types = ["object", "array", "boolean", "int", "integer", "float", "bool", "string"]


# Funkcja sprawdzająca czy typ jest prostym typem JSON
def is_simple_json_type(type_name):
    return type_name in json_simple_types


# Pusta struktura JSON
JsonEmpty = {}
JsonNull = None

# Funkcja inicjalizująca puste struktury JSON
def load_json_empty():
    global JsonEmpty, JsonNull
    JsonEmpty = {}
    JsonNull = None
    return True

JsonEmptyLoaded = load_json_empty()

# Główna funkcja walidacji JSON
def validate_json(obj, schema, object_type, path="", defaults=None):
    if defaults is None:
        defaults = {}

    object_type_is_inline = isinstance(object_type, str)

    # Rekurencyjne rozpatrywanie typu obiektu
    if object_type_is_inline and not is_simple_json_type(object_type):
        return validate_json(obj, schema, schema.get(object_type), path, defaults)

    object_schema = JsonEmpty if object_type_is_inline else object_type
    object_type = object_type if object_type_is_inline else object_schema.get("type", "object")

    # Walidacja obiektu JSON
    if object_type == "object":
        if not isinstance(obj, dict):
            return f"Invalid type: {path} expected: object"
        fields = object_schema.get("fields", object_schema)
        for key, value_schema in fields.items():
            if key not in obj:
                if defaults == {}:
                    return f"Missing key: {path}/{key}"
                obj[key] = defaults.get(key)
            result = validate_json(obj[key], schema, value_schema, f"{path}/{key}", defaults.get(key))
            if result:
                return result

    # Walidacja tablicy JSON
    elif object_type == "array":
        if not isinstance(obj, list):
            return f"Invalid type: {path} expected: array"
        item_schema = object_schema.get("item")
        size = len(obj)
        defaults_size = len(defaults)
        if defaults and size < defaults_size and ("min_length" in object_schema or "length" in object_schema):
            obj.extend(defaults[size:])
            size = len(obj)
        if "max_length" in object_schema and size > object_schema["max_length"]:
            return f"Array too long: {path}"
        if "min_length" in object_schema and size < object_schema["min_length"]:
            return f"Array too short: {path}"
        if "length" in object_schema and size != object_schema["length"]:
            return f"Invalid array size: {path}"
        for i, item in enumerate(obj):
            result = validate_json(item, schema, item_schema, f"{path}/{i}", defaults.get(i if i < defaults_size else defaults_size-1, None))
            if result:
                return result

    # Walidacja typu float
    elif object_type == "float":
        if (not isinstance(obj, float)) and (not isinstance(obj, int)):
            return f"Invalid type: {path} expected: float"
        if not validate_range(obj, object_schema):
            return f"Invalid range: {path}"

    # Walidacja typu integer
    elif object_type == "integer":
        if not isinstance(obj, int):
            return f"Invalid type: {path} expected: integer"
        if not validate_range(obj, object_schema):
            return f"Invalid range: {path}/{path}"

    # Walidacja typu boolean
    elif object_type in ["bool", "boolean"]:
        if not isinstance(obj, bool):
            return f"Invalid type: {path} expected: boolean"

    # Walidacja typu string
    elif object_type in ["string", "text"]:
        if not isinstance(obj, str):
            return f"Invalid type: {path} expected: string"
        if "max_length" in object_schema and len(obj) > object_schema["max_length"]:
            return f"Text too long: {path}"
        if "min_length" in object_schema and len(obj) < object_schema["min_length"]:
            return f"Text too short: {path}"
        if "regexp" in object_schema or "regex" in object_schema:
            pattern = object_schema.get("regexp", object_schema.get("regex"))
            if not re.match(pattern, obj):
                return f"Text does not match the required pattern: {path}"

    return ""


if __name__ == "__main__":

    if len(argv) not in {3, 4}:
        print("Usage:\n\tvalidate_configuration.py <object_filename.json> <schema_filename.json> [type]")
        exit(1)

    obj = json.loads(open(argv[1]).read())
    schema = json.loads(open(argv[2]).read())
    name = 'main' if len(argv) == 3 else argv[3]

    result = validate_json(obj, schema, schema[name])
    if result:
        print(f"Validation failed: {result}")
        exit(1)
    else:
        print("Validation passed")
        exit(0)


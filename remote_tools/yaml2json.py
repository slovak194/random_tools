# %%
import sys

if __name__ == "__main__":
    assert len(sys.argv) == 3
    import yaml
    import json
    config_file_yaml_path = sys.argv[1]
    tmp_json_file_path = sys.argv[2]
    with open(config_file_yaml_path, "r") as yaml_in, open(tmp_json_file_path, "w") as json_out:
        yaml_object = yaml.safe_load(yaml_in)
        json.dump(yaml_object, json_out, indent=" ")

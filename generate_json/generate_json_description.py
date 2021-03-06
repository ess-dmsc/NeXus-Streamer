import numpy as np
import nexusformat.nexus as nexus
import json
import argparse


class NexusToDictConverter:
    """
    Class used to convert nexus format root to python dict
    """

    def __init__(self, truncate_large_datasets: bool = False, large: int = 10):
        """
        :param truncate_large_datasets: if True truncates datasets with any dimension larger than large
        :param large: dimensions larger than this are considered large
        """
        self.truncate_large_datasets = truncate_large_datasets
        self.large = large

    def convert(self, nexus_root: nexus.NXroot) -> dict:
        """
        Converts the given nexus_root to dict
        """
        return {
            "children": [self._root_to_dict(entry)
                         for _, entry in nexus_root.entries.items()]
        }

    def _root_to_dict(self, root: nexus.NXgroup) -> dict:
        try:
            if hasattr(root, 'entries'):
                root_dict = self._handle_group(root)
            else:
                root_dict = self._handle_dataset(root)

            root_dict = self._handle_attributes(root, root_dict)
        except nexus.tree.NeXusError:
            root_dict = {}
        return root_dict

    def truncate_if_large(self, size, data):
        if self.truncate_large_datasets:
            size = list(size)
            for dim_number, dim_size in enumerate(size):
                if dim_size > self.large:
                    size[dim_number] = self.large
            data_copy = np.copy(data[...])
            data_copy.resize(size)
            return data_copy
        return data

    def _get_data_and_type(self, root):
        size = 1
        data = root.nxdata
        dtype = str(root.dtype)
        if isinstance(data, np.ndarray):
            size = data.shape
            data = self.truncate_if_large(size, data)
            if dtype[:2] == '|S':
                data = np.char.decode(data)
            data = data.tolist()
        if dtype[:2] == '|S':
            if not isinstance(data, list):
                data = data.decode('utf-8')
            dtype = 'string'
        elif dtype == "float64":
            dtype = "double"
        elif dtype == "float32":
            dtype = "float"
        return data, dtype, size

    @staticmethod
    def _skip_isis_specific_nodes(root):
        if root.nxclass[:2] == "IX":
            return True
        return False

    def _handle_attributes(self, root, root_dict):
        if root.nxclass and root.nxclass != "NXfield" and root.nxclass != "NXgroup":
            root_dict["attributes"] = [{"name": "NX_class",
                                        "values": root.nxclass}]
        if root.attrs:
            if "attributes" not in root_dict:
                root_dict["attributes"] = []
            root_dict["attributes"] = []
            for attr_name, attr in root.attrs.items():
                data, dtype, size = self._get_data_and_type(attr)
                new_attribute = {"name": attr_name,
                                 "values": data}
                if dtype != "object":
                    new_attribute["type"] = dtype
                root_dict["attributes"].append(new_attribute)
        return root_dict

    def _handle_group(self, root):
        root_dict = {
            "type": "group",
            "name": root.nxname,
            "children": []
        }
        # Add the entries
        entries = root.entries

        if not self._handle_stream(root, root_dict) and not self._skip_isis_specific_nodes(root):
            if entries:
                for entry in entries:
                    child_dict = self._root_to_dict(entries[entry])
                    root_dict["children"].append(child_dict)

        return root_dict

    @staticmethod
    def _get_dtype(root) -> str:
        """
        Get the datatype as a string in the format expected by the file writer
        """
        dtype = str(root.dtype)
        if dtype[:2] == '|S':
            dtype = 'string'
        elif dtype == "float64":
            dtype = "double"
        elif dtype == "float32":
            dtype = "float"
        return dtype

    def _handle_stream(self, root, root_dict: dict) -> bool:
        """
        Insert information to get this data from Kafka stream if it is of an NXclass supported by the NeXus-Streamer
        :param root:
        :param root_dict:
        :return: true if data will be streamed
        """
        stream_info = {}
        is_stream = False
        if isinstance(root, nexus.NXlog):
            stream_info["writer_module"] = "f142"
            is_stream = True
            if root.nxname == 'value_log':
                # For ISIS files the parent of the NXlog has a more useful name
                # which the NeXus-Streamer uses as the source name
                stream_info["source"] = root.nxgroup.nxname
            else:
                stream_info["source"] = root.nxname
            stream_info["topic"] = "SAMPLE_ENV_TOPIC"
            try:
                stream_info["dtype"] = self._get_dtype(root.entries['value'])
            except KeyError:
                try:
                    stream_info["dtype"] = self._get_dtype(root.entries['raw_value'])
                except KeyError:
                    is_stream = False
        elif isinstance(root, nexus.NXevent_data):
            stream_info["writer_module"] = "ev42"
            stream_info["topic"] = "EVENT_DATA_TOPIC"
            stream_info["source"] = "NeXus-Streamer"
            is_stream = True
        if is_stream:
            root_dict["children"].append({
                "type": "stream",
                "stream": stream_info
            })
        return is_stream

    def _handle_dataset(self, root):
        data, dataset_type, size = self._get_data_and_type(root)
        root_dict = {
            "type": "dataset",
            "name": root.nxname,
            "dataset": {
                "type": dataset_type
            },
            "values": data
        }
        if size != 1:
            root_dict["dataset"]["size"] = size

        return root_dict


def object_to_json_file(tree_dict, filename):
    """
    Create a JSON file describing the NeXus file
    WARNING, output files can easily be 10 times the size of input NeXus file

    :param tree_dict: Root node of the tree
    :param filename: Name for the output file
    """
    with open(filename, 'w') as outfile:
        json.dump(tree_dict, outfile, indent=2, sort_keys=False)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "-i", "--input-filename", type=str, help="Input NeXus file to generate JSON description of"
    )
    parser.add_argument("-o", "--output-filename", type=str, help="Output filename for the NeXus structure JSON")
    args = parser.parse_args()
    converter = NexusToDictConverter(truncate_large_datasets=True)

    nexus_file = nexus.nxload(args.input_filename)
    tree = converter.convert(nexus_file)
    object_to_json_file(tree, args.output_filename)

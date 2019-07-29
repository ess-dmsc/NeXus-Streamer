# Generate JSON description of NeXus file

Matches the `nexus_structure` field of file writer start messages, documented here: https://github.com/ess-dmsc/kafka-to-nexus/blob/master/documentation/commands.md

The included python script can be used to automatically generate a JSON file: [generate_json_description.py](generate_json_description.py)
This requires Python 3.6+, dependencies can be installed with pip: 
```
pip install -r requirements.txt
```

and run, for example, with:
```
python generate_json_description.py --input-filename ../data/SANS2D_minimal.nxs --output-filename SANS2D_minimal_json.txt
```

If using via docker-compose note the `SEND_GEOMETRY` option in [docker-compose.yml](docker-compose.yml), set to 1 to automatically generate the JSON description of the NeXus file and include this in the run start message sent to Mantid.

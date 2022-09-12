import base64
import random
import argparse
import sys
import urllib
import urllib.parse

from serialization import marshal
from deserialization import unmarshal
from exceptions import SerializationError, DeserializationError


#invariant->a string can handle any ascii values in any order without raising any errors
#if strings are not percent encoded correctly, any reserved characters such as ,{}%00 would create an error
def parse_cmd_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
            '--count',
            type=int,
            required=True,
            dest='count',
            metavar='n',
            help='The number of inputs to generate and test.',
            )
    parser.add_argument(
            '--seed',
            type=str,
            required=True,
            dest='seed',
            metavar='seed_string',
            help='The seed to generate inputs from.',
            )
    args = parser.parse_args()
    return args.seed, args.count

#create a random string out of any ascii characters
def create_values_string(times):
    value = ''
    for i in range(times):
        value += chr(random.randint(0,255))

    return value


def main():
    seed, count = parse_cmd_args()
    random.seed(seed)

    all_inputs_handle = open(seed+'.inputs', 'w')
    error_inputs_handle = open(seed+'.failure', 'w')


    for i in range(count):    
        test_value = create_values_string(count)
            
        line_for_files = '{input:'+ test_value +'}\n'
        all_inputs_handle.write(line_for_files)
        did_error = False
            
        #if an input is marshalled and unmarshalled you should get the ouput should match the input
        got = unmarshal(marshal({'a':test_value}))
        if got != {'a':test_value}:
            id_error = True
            
        if did_error:
             error_inputs_handle.write(line_for_files)

    all_inputs_handle.close()
    error_inputs_handle.close()
    sys.exit(0)

main()

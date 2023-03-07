import grpc
import datacontract_pb2_grpc
import datacontract_pb2
import time
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.serialization import load_pem_private_key
import time
import sys
import mysql.connector
import read_logic
import pathlib
from datetime import datetime
import re
import ctypes
import os
import hashlib
import subprocess

def main(data_contract_name, parameters):

    request = datacontract_pb2.DataContractRequest(data_contract_name=data_contract_name, parameters=parameters)
    response = stub.ExecuteDataContract(request)
    # connect to mysql to insert result into trustdble.data_contract_result table
    connection = mysql.connector.connect(host="localhost",user="root", password="")

    sql_select_Query = "SELECT logic FROM trustdble.remote_data_contracts WHERE name=\"{}\"".format((data_contract_name))
    cursor = connection.cursor()
    cursor.execute(sql_select_Query)
    logic = cursor.fetchone()[0]
    data_contract_logic_hash=  hashlib.sha256(logic.encode()).hexdigest()
    # Fetch SGx measurements from remote_data_contracts table and compare them with SGX measuremnets send by the owner(grpc server)
    SGX_sql_select_Query = "SELECT MRENCLAVE,MRSIGNER,isv_prod_id,isv_svn from trustdble.remote_data_contracts WHERE name=\"{}\"".format((data_contract_name))
    SGX_cursor = connection.cursor()
    SGX_cursor.execute(SGX_sql_select_Query)
    SGX_measurements = SGX_cursor.fetchone()
    #assign values to expexted sgx measurements
    expected_mrenclave=SGX_measurements[0]
    expected_mrsigner=SGX_measurements[1]
    expected_isv_prod_id=SGX_measurements[2]
    expected_isv_svn=SGX_measurements[3]

    # compare the expected hash value of data contract logic and the one sent by the owner (grpc server)
    if(data_contract_logic_hash== response.data_contract_logic_hash):
        # compare the expected SGX measurements and the ones sent by the owner (grpc server)
        if(expected_mrenclave== response.MRENCLAVE and expected_mrsigner== response.MRSIGNER and expected_isv_prod_id== response.isv_prod_id and expected_isv_svn== response.isv_svn):
            sql_create_Query = "CREATE TABLE IF NOT EXISTS trustdble.data_contracts_result (id INT NOT NULL AUTO_INCREMENT, data_contract_name VARCHAR(100), result LONGTEXT, time DATETIME , PRIMARY KEY (id) )"
            cursor.execute(sql_create_Query)
            # get Time to insert into the data_contracts_result
            now = datetime.now()
            current_formated_date = now.strftime('%Y-%m-%d %H:%M:%S')
            # ignore escape characters
            re.escape(response.result)
            # insert the result of data contract into table data_contracts_result
            sql_insert_Query = """INSERT INTO trustdble.data_contracts_result(data_contract_name, result, time) values (%(dc_name)s,%(result)s,%(time)s);"""
            data_result = {'dc_name':data_contract_name, 'result':response.result,'time':current_formated_date}
            cursor.execute(sql_insert_Query,data_result)
            connection.commit()
        else:
            print("SGX mesurements does not macth the expected SGX measurements ")
    else:
            print("The executed data contract logic is not the same as expected")
    if connection.is_connected():
      connection.close()
      cursor.close()
      SGX_cursor.close()
if __name__ == '__main__':
    connection = mysql.connector.connect(host="localhost",user="root", password="")
    # Fetch SGx measurements from remote_data_contracts table and compare them with SGX measuremnets send by the owner(grpc server)
    SGX_sql_select_Query = "SELECT MRENCLAVE,MRSIGNER,isv_prod_id,isv_svn from trustdble.remote_data_contracts WHERE name=\"{}\"".format(str(sys.argv[1]))
    SGX_cursor = connection.cursor()
    SGX_cursor.execute(SGX_sql_select_Query)
    SGX_measurements = SGX_cursor.fetchone()
    #assign values to expexted sgx measurements
    expected_mrenclave=SGX_measurements[0]
    expected_mrsigner=SGX_measurements[1]
    expected_isv_prod_id=SGX_measurements[2]
    expected_isv_svn=SGX_measurements[3]
    if(expected_mrenclave=="0" and expected_mrsigner=="0" and expected_isv_prod_id=="0" and expected_isv_svn=="0"):
        sgx_supported = False
    else:
        sgx_supported = True
    if(sgx_supported==True):
        # load libra_tls_verify_dcap library to load the functions ra_tls_verify_callback_der and ra_tls_set_measurement_callback
        ra_tls_verify_lib = ctypes.CDLL(os.path.abspath("/usr/local/lib/x86_64-linux-gnu/gramine/runtime/glibc/libra_tls_verify_dcap.so"))
        ra_tls_verify_callback_f = ra_tls_verify_lib.ra_tls_verify_callback_der
        # this function is doing nothing here as my_verify_measurements always returns 0, we added it to avoid warnings
        ra_tls_set_measurement_callback_f = ra_tls_verify_lib.ra_tls_set_measurement_callback

        # Define the C type of the callback function
        CALLBACKFUNC = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p)

        # Create a Python wrapper for  callback function which always returns 0
        def my_verify_measurements(mrenclave, mrsigner, isv_prod_id, isv_svn):
            return 0
        # Create a Python wrapper for  callback function
        def verify_certificate(crt,crt_size) -> int:
            return ra_tls_verify_callback_f(crt, crt_size)

        # Convert the python function to a CFunctionType instance (only for avoiding warnings)
        my_verify_measurements_callback = CALLBACKFUNC(my_verify_measurements)

        # Pass the CFunctionType instance to the C function (only for avoiding warnings)
        ra_tls_set_measurement_callback_f(my_verify_measurements_callback)

        current_path=pathlib.Path().resolve()
        current_path_str= '/'.join(str(current_path).rsplit('/', 1)[:-1])
        path_to_cert=str(current_path_str)+"/build-debug/plugin/data_contracts/remote_server_cert.pem"
        # Load the server's certificate
        with open(path_to_cert, 'rb') as f:
            remote_server_cert = f.read()
        # Load the certificate from PEM data
        cert = x509.load_pem_x509_certificate(remote_server_cert, default_backend())

        # Convert the certificate to DER format
        der_data = cert.public_bytes(encoding=serialization.Encoding.DER)
        # call verify_certificate to verify the server's certificate and measurements
        ret=verify_certificate(der_data,len(der_data))
        if(ret==0):
            # connect to mysql to fetch server address from data_contracts table refer to requested data contract name
            connection = mysql.connector.connect(host="localhost",user="root", password="")
            sql_select_Query = "SELECT server_address from trustdble.remote_data_contracts WHERE name=\"{}\"".format(str(sys.argv[1]))
            cursor = connection.cursor()
            cursor.execute(sql_select_Query)
            server_address = cursor.fetchone()
            connection.close()
            cursor.close()
            if(server_address!=None):
                server_address_str = ' '.join([str(element) for element in server_address ])
                # get current path
                current_path=pathlib.Path().resolve()
                current_path_str=str(current_path).replace('data','')
                path_to_cert=str(current_path_str)+"/keys/tdb-server-cert.pem"
                path_to_key=str(current_path_str)+"/keys/tdb-server-key.pem"
                # Fetch the private's key password
                start = str(current_path).index("_home") + len("_home")
                end = str(current_path).index("data")
                server_number = str(current_path)[start:end]
                password=server_number+"tdb-server-key"
                with open(path_to_key, 'rb') as f:
                    private_key=load_pem_private_key(f.read(), password=password.encode(),
                                                  backend=default_backend())
                private_key_bytes =private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.TraditionalOpenSSL,
                encryption_algorithm=serialization.NoEncryption()
                )
                with open(path_to_cert, 'rb') as f:
                    client_certificate = f.read()
                # create credentials and secure channel
                creds = grpc.ssl_channel_credentials(remote_server_cert,private_key_bytes,client_certificate)
                cert_cn = "RATLS"
                options = (('grpc.ssl_target_name_override', cert_cn,),)
                channel = grpc.secure_channel(server_address_str+":50051", creds,options)
                stub = datacontract_pb2_grpc.ExecDataContractStub(channel)
                main(sys.argv[1], sys.argv[2])

            else:
                print("gRPC_client: the specified data contract does not exist",flush=True)
        else:
            print(" failed! certificate verification returned ", ret)
    else:
        # connect to mysql to fetch server address from data_contracts table refer to requested data contract name
        connection = mysql.connector.connect(host="localhost",user="root", password="")
        sql_select_Query = "SELECT server_address from trustdble.remote_data_contracts WHERE name=\"{}\"".format(str(sys.argv[1]))
        cursor = connection.cursor()
        cursor.execute(sql_select_Query)
        server_address = cursor.fetchone()
        connection.close()
        cursor.close()
        if(server_address!=None):
            server_address_str = ' '.join([str(element) for element in server_address ])
            # get current path
            current_path=pathlib.Path().resolve()
            current_path_str=str(current_path).replace('data','')
            path_to_cert=str(current_path_str)+"/keys/tdb-server-cert.pem"
            path_to_key=str(current_path_str)+"/keys/tdb-server-key.pem"
            # Fetch the private's key password
            start = str(current_path).index("_home") + len("_home")
            end = str(current_path).index("data")
            server_number = str(current_path)[start:end]
            password=server_number+"tdb-server-key"
            with open(path_to_key, 'rb') as f:
                private_key=load_pem_private_key(f.read(), password=password.encode(),
                                                  backend=default_backend())
                private_key_bytes =private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.TraditionalOpenSSL,
                encryption_algorithm=serialization.NoEncryption()
                )
            with open(path_to_cert, 'rb') as f:
                client_certificate = f.read()
            current_path_str=os.path.dirname(os.path.dirname(str(current_path)))
            path_to_ca=str(current_path_str)+"/src/build-debug/plugin/data_contracts/trustdble_ca.crt"
            with open(path_to_ca, 'rb') as f:
                ca = f.read()

            cert_cn = "localhost"
            options = (('grpc.ssl_target_name_override', cert_cn,),)
            creds = grpc.ssl_channel_credentials(ca,private_key_bytes,client_certificate)
            channel = grpc.secure_channel(server_address_str+":50051", creds,options)
            stub = datacontract_pb2_grpc.ExecDataContractStub(channel)
            main(sys.argv[1], sys.argv[2])
#!/usr/bin/env python3
"""The Python implementation of the GRPC server."""

from concurrent import futures
import logging

import grpc
import datacontract_pb2
import datacontract_pb2_grpc
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.serialization import load_pem_private_key
from cryptography.hazmat.primitives.serialization import load_pem_public_key
from cryptography.hazmat.primitives.asymmetric import dsa, rsa
import time 
import sys
import mysql.connector
import imp
import read_logic
imp.reload(read_logic)
import os
import pathlib
import ctypes
from ctypes import POINTER
from ctypes import c_ubyte
from ctypes import *
import hashlib
import socket

class DataContractExecutionImp(datacontract_pb2_grpc.ExecDataContractServicer):
 def ExecuteDataContract(self,request:datacontract_pb2.DataContractRequest,context): 
 # call to Read the logic related to specific id
  read_logic.ReadLogic(request.data_contract_name)
   # import python script DataContractLogic.py
  import DataContractLogic
  imp.reload(DataContractLogic)
  
  # Check if logic needs input parameters
  if request.parameters=="None":
 #run DataContractLogic main method
   logic_result=DataContractLogic.main()
  else:
    logic_result=DataContractLogic.main(request.parameters)
  with open ("DataContractLogic.py","rb") as f:
    data_contract_logic=f.read()
  data_contract_logic_hash = hashlib.sha256(data_contract_logic).hexdigest()

  connection = mysql.connector.connect(host="localhost",user="root", password="")
  sql_select_Query = "SELECT * from trustdble.enclave_measurement"
  cursor = connection.cursor()
  cursor.execute(sql_select_Query)
  SGX_measurements = cursor.fetchone()
  MRSIGNER = SGX_measurements[0]
  MRENCLAVE = SGX_measurements[1]
  isv_prod_id = SGX_measurements[2]
  isv_svn = SGX_measurements[3]
  connection.close()
  cursor.close()
  return datacontract_pb2.DataContractReply(data_contract_name=request.data_contract_name,result=str(logic_result), data_contract_logic_hash=data_contract_logic_hash,MRENCLAVE=MRENCLAVE, MRSIGNER=MRSIGNER,isv_prod_id=isv_prod_id,isv_svn=isv_svn)
  
if __name__ == '__main__':
    if(sys.argv[1]=="0"):
      # check if RA-TLS key and certificate is created before to avoid recreating it
      if(os.path.exists("server_cert.pem")==False and os.path.exists("server_key.pem")==False):
        # calling ra_tls_create_key_and_crt_der from library libra_tls_attest
        libra_tls_attest = ctypes.CDLL(os.path.abspath("/usr/local/lib/x86_64-linux-gnu/gramine/runtime/glibc/libra_tls_attest.so"))
        libra_tls_attest.ra_tls_create_key_and_crt_der.argtypes = (ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)), ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)), ctypes.POINTER(ctypes.c_size_t))
        libra_tls_attest.ra_tls_create_key_and_crt_der.restype = ctypes.c_int

        # Create the input and output variables
        der_key_size = ctypes.c_size_t()
        der_crt_size = ctypes.c_size_t()
        der_key = ctypes.POINTER(ctypes.c_uint8)()
        der_crt = ctypes.POINTER(ctypes.c_uint8)()
        # Call the function
        ret=libra_tls_attest.ra_tls_create_key_and_crt_der(ctypes.byref(der_key), 
            ctypes.byref(der_key_size), ctypes.byref(der_crt), ctypes.byref(der_crt_size))
        # Check the return value
        if ret != 0:
            print("Error: ra_tls_create_key_and_crt_der returned", ret)
        else:
            # convert der_cert and der_key to bytes
            cert_bytes = bytes(ctypes.string_at(der_crt, der_crt_size.value))
            key_bytes = bytes(ctypes.string_at(der_key, der_key_size.value))

            # Use the output variables
            cert = x509.load_der_x509_certificate(cert_bytes, backend=default_backend())
            pem_cert = cert.public_bytes(encoding=serialization.Encoding.PEM)
            # write RA-TLS certificate into server_cert.pem
            f = open("server_cert.pem", "w")
            f.write(pem_cert.decode())
            f.close()
            key = serialization.load_der_private_key(key_bytes, None, default_backend())
            pem_key = key.private_bytes(encoding=serialization.Encoding.PEM,format=serialization.PrivateFormat.PKCS8,encryption_algorithm=serialization.NoEncryption()) 
            f = open("server_key.pem", "w")
            f.write(pem_key.decode())
            f.close()
      # if RA-TLS certficate and key are created before use them
      else:
        with open("server_cert.pem", 'rb') as f:
            pem_cert=f.read()  
        with open("server_key.pem", 'rb') as f:
            private_key = load_pem_private_key(f.read(), password=None, backend=default_backend())
        pem_key = private_key.private_bytes(encoding=serialization.Encoding.PEM, format=serialization.PrivateFormat.PKCS8, encryption_algorithm=serialization.NoEncryption())
        
    else:
        # open trustdble server's private key and certificate
        with open(str(sys.argv[2]), 'rb') as f:
            private_key=load_pem_private_key(f.read(), password=str(sys.argv[3]).encode(),
                                                  backend=default_backend())
        pem_key =private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=serialization.NoEncryption()
        )
        with open(str(sys.argv[4]), 'rb') as f:
            pem_cert = f.read()
    with open("trustdble_ca.crt", 'rb') as f:
        ca = f.read()
    
    # create a gRPC server
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    # create gRPC server credentials
    server_credentials = grpc.ssl_server_credentials(
        ((pem_key, pem_cert), ),ca)

    # add the servicer created above to the server
    datacontract_pb2_grpc.add_ExecDataContractServicer_to_server(DataContractExecutionImp(), server)
    # listen on port 50051
    print('Starting gRPC server. Listening on port 50051.',flush=True)
    hostname = socket.gethostname()
    ip_address = socket.gethostbyname(hostname)
    server.add_secure_port(ip_address+':50051', server_credentials)
    server.start()
    # since server.start() will not block,
    # a sleep-loop is added to keep alive
    try:
        while True:
            time.sleep(86400)
    except KeyboardInterrupt:
        server.stop(0)
import hashlib
import mysql.connector
import sys
import subprocess

def main(port_number):
    
    result = subprocess.run(['grep', '-q', '-w', 'sgx', '/proc/cpuinfo'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode == 0:
        sgx_supported = True
    else:
        sgx_supported = False
    if(sgx_supported==True):
        from graminelibos import Sigstruct
        with open("python.sig", mode='rb') as file:
            sig = Sigstruct.from_bytes(file.read())
        mrsigner = hashlib.sha256()
        mrsigner.update(sig['modulus'])
        mrsigner = mrsigner.hexdigest()
        mrenclave=sig['enclave_hash'].hex()
        isv_prod_id=sig['isv_prod_id']
        isv_svn=sig['isv_svn']
    else:
        mrsigner="0"
        mrenclave="0"
        isv_prod_id="0"
        isv_svn="0"
    connection = mysql.connector.connect(host="localhost",user="root", password="",port=port_number)
    sql_create_Query = "CREATE TABLE IF NOT EXISTS trustdble.enclave_measurement (MRSIGNER  VARCHAR(64), MRENCLAVE  VARCHAR(64), isv_prod_id VARCHAR(2), isv_svn VARCHAR(2))" 
    cursor = connection.cursor()
    cursor.execute(sql_create_Query)
    sql_insert_Query = """INSERT INTO trustdble.enclave_measurement(MRSIGNER, MRENCLAVE, isv_prod_id,isv_svn) values (%s,%s,%s,%s);"""
    cursor.execute(sql_insert_Query, (mrsigner, mrenclave, isv_prod_id, isv_svn))
    connection.commit()
    if connection.is_connected():
        connection.close()
        cursor.close()
if __name__ == "__main__":
 main(sys.argv[1])
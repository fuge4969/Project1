#include "seal/seal.h"
#include "common.h"
#include <iostream>
#include <sstream>

using namespace std;
using namespace seal;
using namespace asio_net;

int main() {
    asio::io_context io;
    ClientConnection client(io, "127.0.0.1", 9000);

    // ���չ�Կ
    auto pk_buf = client.recv();
    stringstream pk_stream(string(pk_buf.begin(), pk_buf.end()));

    // �������ã��������˱���һ�£�
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(2048);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(2048));
    parms.set_plain_modulus(PlainModulus::Batching(2048, 20));
    SEALContext context(parms);

    PublicKey public_key;
    public_key.load(context, pk_stream);

    Encryptor encryptor(context, public_key);
    BatchEncoder encoder(context);

    int input;
    cout << "[Client] �������������: ";
    cin >> input;

    vector<uint64_t> data(encoder.slot_count(), 0ULL);
    data[0] = static_cast<uint64_t>(input);

    Plaintext pt;
    encoder.encode(data, pt);

    Ciphertext ct;
    encryptor.encrypt(pt, ct);

    // ��������
    stringstream ct_stream;
    ct.save(ct_stream);
    string ct_str = ct_stream.str();
    client.send(vector<char>(ct_str.begin(), ct_str.end()));

    // ���ձȽϽ��
    auto resp = client.recv();
    string result(resp.begin(), resp.end());
    cout << "[Client] ���������ؽ��: " << result << endl;

    return 0;
}

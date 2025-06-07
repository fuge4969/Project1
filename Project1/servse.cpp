#include "seal/seal.h"
#include "common.h"
#include <iostream>
#include <thread>

using namespace std;
using namespace seal;
using namespace asio_net;

int decode_signed(uint64_t val, uint64_t mod)
{
    if (val > mod / 2) return static_cast<int>(val - mod);
    return static_cast<int>(val);
}

int main() {
    asio::io_context io;
    ServerConnection server(io, 9000);
    cout << "[Server] �ȴ������ͻ�������..." << endl;

    tcp::socket sock1 = std::move(server.accept());
    cout << "[Server] �ͻ��� 1 ������" << endl;
    tcp::socket sock2 = std::move(server.accept());
    cout << "[Server] �ͻ��� 2 ������" << endl;

    // SEAL �������ã�����ͻ��˱���һ�£�
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(2048);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(2048));
    parms.set_plain_modulus(PlainModulus::Batching(2048, 20));
    SEALContext context(parms);

    KeyGenerator keygen(context);
    PublicKey public_key;
    keygen.create_public_key(public_key);
    SecretKey secret_key = keygen.secret_key();
    Decryptor decryptor(context, secret_key);
    BatchEncoder encoder(context);

    // ���͹�Կ���ͻ���
    stringstream pk_stream;
    public_key.save(pk_stream);
    string pk_str = pk_stream.str();
    server.send(sock1, vector<char>(pk_str.begin(), pk_str.end()));
    server.send(sock2, vector<char>(pk_str.begin(), pk_str.end()));

    // ������������
    auto buf1 = server.recv(sock1);
    auto buf2 = server.recv(sock2);

    Ciphertext enc1, enc2;
    enc1.load(context, reinterpret_cast<const seal::seal_byte*>(buf1.data()), buf1.size());
    enc2.load(context, reinterpret_cast<const seal::seal_byte*>(buf2.data()), buf2.size());

    // ���� & �Ƚ�
    Plaintext pt1, pt2;
    decryptor.decrypt(enc1, pt1);
    decryptor.decrypt(enc2, pt2);

    vector<uint64_t> vec1, vec2;
    encoder.decode(pt1, vec1);
    encoder.decode(pt2, vec2);

    uint64_t mod = parms.plain_modulus().value();
    int val1 = decode_signed(vec1[0], mod);
    int val2 = decode_signed(vec2[0], mod);

    cout << "[Server] �ͻ���1: " << val1 << ", �ͻ���2: " << val2 << endl;

    string result;
    if (val1 > val2) result = "�ͻ���1 > �ͻ���2";
    else if (val1 < val2) result = "�ͻ���1 < �ͻ���2";
    else result = "�ͻ���1 == �ͻ���2";

    // �ظ����
    vector<char> result_data(result.begin(), result.end());
    server.send(sock1, result_data);
    server.send(sock2, result_data);

    cout << "[Server] �ȽϽ���ѷ��͡�" << endl;
    return 0;
}

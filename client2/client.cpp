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

    // 接收公钥
    auto pk_buf = client.recv();
    stringstream pk_stream(string(pk_buf.begin(), pk_buf.end()));

    // 参数设置（需与服务端保持一致）
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
    cout << "[Client] 请输入你的数字: ";
    cin >> input;

    vector<uint64_t> data(encoder.slot_count(), 0ULL);
    data[0] = static_cast<uint64_t>(input);

    Plaintext pt;
    encoder.encode(data, pt);

    Ciphertext ct;
    encryptor.encrypt(pt, ct);

    // 发送密文
    stringstream ct_stream;
    ct.save(ct_stream);
    string ct_str = ct_stream.str();
    client.send(vector<char>(ct_str.begin(), ct_str.end()));

    // 接收比较结果
    auto resp = client.recv();
    string result(resp.begin(), resp.end());
    cout << "[Client] 服务器返回结果: " << result << endl;

    return 0;
}

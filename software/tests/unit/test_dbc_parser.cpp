// test_dbc_parser.cpp - L3 protocol: DBC parsing + signal decoding
#include <string>
#include <vector>

#include "core/bus_frame.h"
#include "protocol/can/can_decoder.h"
#include "protocol/dbc/dbc_parser.h"
#include "test_framework.h"

using namespace bt;

namespace {

const char* kDbcText = R"(
VERSION "1.0"

NS_ :
	NS_DESC_
	CM_

BS_:

BU_: Vector__XXX ECU1 ECU2

BO_ 100 EngineData: 8 Vector__XXX
 SG_ EngineSpeed : 8|16@1+ (0.25,0) [0|16000] "rpm" ECU1
 SG_ EngineTemp : 24|8@0+ (1,-40) [-40|215] "degC" ECU1

BO_ 200 VehicleSpeed: 8 ECU1
 SG_ VehicleSpeed : 0|8@1- (0.01,0) [0|300] "km/h" ECU2

CM_ BO_ 100 "engine block";
BA_ "BusType" "CAN";
)";

}  // namespace

BT_TEST(dbc_parse_basic) {
    DbcParser p;
    std::string err;
    BT_CHECK(p.parse(kDbcText, &err));
    BT_CHECK_EQ(p.message_count(), 2U);

    const DbcMessage* m = p.find_message(100U);
    BT_CHECK(m != nullptr);
    BT_CHECK_EQ(m->name, std::string("EngineData"));
    BT_CHECK_EQ(m->dlc, 8U);
    BT_CHECK_EQ(m->signals.size(), 2U);

    const DbcMessage* v = p.find_message(std::string("VehicleSpeed"));
    BT_CHECK(v != nullptr);
    BT_CHECK_EQ(v->id, 200U);
    BT_CHECK_EQ(v->signals.size(), 1U);
    return true;
}

BT_TEST(dbc_signal_geometry) {
    DbcParser p;
    BT_CHECK(p.parse(kDbcText));
    const DbcMessage* m = p.find_message(100U);
    BT_CHECK(m != nullptr);

    const Signal& s = m->signals[0];
    BT_CHECK_EQ(s.name, std::string("EngineSpeed"));
    BT_CHECK_EQ(s.start_bit, 8U);
    BT_CHECK_EQ(s.length, 16U);
    BT_CHECK(!s.little_endian);  // @1+ = Motorola
    BT_CHECK(!s.is_signed);
    BT_CHECK_EQ(s.factor, 0.25);
    BT_CHECK_EQ(s.offset, 0.0);
    BT_CHECK_EQ(s.min_value, 0.0);
    BT_CHECK_EQ(s.max_value, 16000.0);
    BT_CHECK_EQ(s.unit, std::string("rpm"));
    return true;
}

BT_TEST(dbc_decode_intel_signal) {
    DbcParser p;
    BT_CHECK(p.parse(kDbcText));
    const DbcMessage* m = p.find_message(100U);
    BT_CHECK(m != nullptr);

    uint8_t d[8] = {0};
    d[3] = 0x96;  // EngineTemp @24|8@0+ -> raw 150 -> 150-40 = 110 degC
    BusFrame f =
        make_frame(BT_BUS_CAN, 0, BT_DIR_RX, 100U, false, false, 8, d, 0);
    std::vector<SignalValue> out;
    BT_CHECK(CanDecoder::decode_frame(*m, f, &out));
    BT_CHECK_EQ(out.size(), 2U);
    BT_CHECK_EQ(out[1].signal_name, std::string("EngineTemp"));
    BT_CHECK_EQ(out[1].raw, 0x96U);
    BT_CHECK_EQ(out[1].physical, 110.0);
    return true;
}

BT_TEST(dbc_decode_motorola_signal) {
    DbcParser p;
    BT_CHECK(p.parse(kDbcText));
    const DbcMessage* m = p.find_message(100U);
    BT_CHECK(m != nullptr);

    uint8_t d[8] = {0};
    d[1] = 0x12;
    d[2] = 0x34;  // EngineSpeed @8|16@1+ -> raw 0x1234 = 4660 -> 1165.0 rpm
    BusFrame f =
        make_frame(BT_BUS_CAN, 0, BT_DIR_RX, 100U, false, false, 8, d, 0);
    std::vector<SignalValue> out;
    BT_CHECK(CanDecoder::decode_frame(*m, f, &out));
    BT_CHECK_EQ(out[0].signal_name, std::string("EngineSpeed"));
    BT_CHECK_EQ(out[0].raw, 0x1234U);
    BT_CHECK_EQ(out[0].physical, 1165.0);
    return true;
}

BT_TEST(dbc_decode_signed_signal) {
    DbcParser p;
    BT_CHECK(p.parse(kDbcText));
    const DbcMessage* m = p.find_message(200U);
    BT_CHECK(m != nullptr);

    uint8_t d[8] = {0xFF, 0, 0, 0, 0, 0, 0, 0};
    // VehicleSpeed @0|8@1- -> raw 0xFF, sign-extended to -1 -> -0.01 km/h
    BusFrame f =
        make_frame(BT_BUS_CAN, 0, BT_DIR_RX, 200U, false, false, 8, d, 0);
    std::vector<SignalValue> out;
    BT_CHECK(CanDecoder::decode_frame(*m, f, &out));
    BT_CHECK_EQ(out.size(), 1U);
    BT_CHECK_EQ(out[0].signal_name, std::string("VehicleSpeed"));
    BT_CHECK_EQ(out[0].raw, 0xFFFFFFFFFFFFFFFFULL);
    BT_CHECK_EQ(out[0].physical, -0.01);
    return true;
}

BT_TEST(dbc_sign_extend_helper) {
    BT_CHECK_EQ(CanDecoder::sign_extend(0x00U, 8U), 0x00ULL);
    BT_CHECK_EQ(CanDecoder::sign_extend(0x7FU, 8U), 0x7FULL);
    BT_CHECK_EQ(CanDecoder::sign_extend(0x80U, 8U), 0xFFFFFFFFFFFFFF80ULL);
    BT_CHECK_EQ(CanDecoder::sign_extend(0xFFU, 8U), 0xFFFFFFFFFFFFFFFFULL);
    BT_CHECK_EQ(CanDecoder::sign_extend(0x1234U, 16U), 0x1234ULL);
    BT_CHECK_EQ(CanDecoder::sign_extend(0x8000U, 16U), 0xFFFFFFFFFFFF8000ULL);
    return true;
}

BT_TEST(dbc_rejects_bad_input) {
    DbcParser p;
    std::string err;
    BT_CHECK(!p.parse("BO_ notanumber Msg: 8\n", &err));
    BT_CHECK(!err.empty());
    BT_CHECK(!p.parse("SG_ X : 0|8@0+ (1,0)\n", &err));  // SG_ before BO_
    BT_CHECK_EQ(p.message_count(), 0U);
    return true;
}

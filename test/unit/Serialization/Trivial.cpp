
#include "caffeine/Protos/operation.capnp.h"

#include <capnp/message.h>
#include <capnp/serialize.h>

#include <gtest/gtest.h>

using ::capnp::MallocMessageBuilder;
using ::capnp::messageToFlatArray;
using ::capnp::readMessageCopy;

class CapnpTrivialTest : public ::testing::Test {};

TEST_F(CapnpTrivialTest, can_use_library) {
  MallocMessageBuilder pack_message;

  auto symbol = pack_message.initRoot<caffeine::protos::Symbol>();

  symbol.initName(25);
  symbol.setName("Teeeesting");

  auto packed = messageToFlatArray(pack_message);

  auto input_stream = kj::ArrayInputStream(packed.asBytes());
  MallocMessageBuilder unpack_message;
  readMessageCopy(input_stream, unpack_message);

  auto unpacked_obj = unpack_message.getRoot<caffeine::protos::Symbol>();

  ASSERT_STREQ(unpacked_obj.getName().begin(), "Teeeesting");
}

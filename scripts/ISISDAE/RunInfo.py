# automatically generated, do not modify

# namespace: ISISDAE

import flatbuffers

class RunInfo(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsRunInfo(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = RunInfo()
        x.Init(buf, n + offset)
        return x


    # RunInfo
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # RunInfo
    def StartTime(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint64Flags, o + self._tab.Pos)
        return 0

    # RunInfo
    def RunNumber(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    # RunInfo
    def InstName(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return ""

    # RunInfo
    def StreamOffset(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int64Flags, o + self._tab.Pos)
        return 0

def RunInfoStart(builder): builder.StartObject(4)
def RunInfoAddStartTime(builder, startTime): builder.PrependUint64Slot(0, startTime, 0)
def RunInfoAddRunNumber(builder, runNumber): builder.PrependInt32Slot(1, runNumber, 0)
def RunInfoAddInstName(builder, instName): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(instName), 0)
def RunInfoAddStreamOffset(builder, streamOffset): builder.PrependInt64Slot(3, streamOffset, 0)
def RunInfoEnd(builder): return builder.EndObject()

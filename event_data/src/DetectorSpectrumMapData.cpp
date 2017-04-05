#include "DetectorSpectrumMapData.h"
#include <fstream>
#include <sstream>

bool file_exists(const std::string &name) {
  if (FILE *file = fopen(name.c_str(), "r")) {
    fclose(file);
    return true;
  } else {
    return false;
  }
}

DetectorSpectrumMapData::DetectorSpectrumMapData(const std::string &filename) {
  if (!file_exists(filename))
    throw std::runtime_error(
        filename +
        " cannot be found, please check full path is given and correct.");
  readFile(filename);
}

void DetectorSpectrumMapData::readFile(const std::string &filename) {
  std::ifstream infile(filename);
  std::string line;
  std::getline(infile, line); // discard first line
  std::getline(infile, line); // line with the number of entries
  std::istringstream iss(line);
  iss >> m_numberOfEntries;
  m_detectors.resize(static_cast<size_t>(m_numberOfEntries));
  m_spectra.resize(static_cast<size_t>(m_numberOfEntries));
  std::getline(infile, line); // discard third line
  size_t entryNumber = 0;
  while (std::getline(infile, line)) {
    std::istringstream iss(line);
    if (!(iss >> m_detectors[entryNumber] >> m_spectra[entryNumber])) {
      break;
    } // error

    entryNumber++;
  }
}

void DetectorSpectrumMapData::decodeMessage(const uint8_t *buf) {
  auto messageData = ISISStream::GetSpectraDetectorMapping(buf);

  auto detFBVector = messageData->det();
  auto specFBVector = messageData->spec();
  setNumberOfEntries(messageData->n_spectra());
  m_detectors.resize(static_cast<size_t>(m_numberOfEntries));
  m_spectra.resize(static_cast<size_t>(m_numberOfEntries));
  std::copy(detFBVector->begin(), detFBVector->end(), m_detectors.begin());
  std::copy(specFBVector->begin(), specFBVector->end(), m_spectra.begin());
}

flatbuffers::unique_ptr_t
DetectorSpectrumMapData::getBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto messageFlatbuf = ISISStream::CreateSpectraDetectorMapping(
      builder, builder.CreateVector(m_spectra),
      builder.CreateVector(m_detectors), m_numberOfEntries);
  builder.Finish(messageFlatbuf);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}

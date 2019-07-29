#include <flatbuffers/flatbuffers.h>
#include <fstream>
#include <sstream>

#include "DetectorSpectrumMapData.h"

bool file_exists(const std::string &name) {
  if (FILE *file = fopen(name.c_str(), "r")) {
    fclose(file);
    return true;
  }
  return false;
}

DetectorSpectrumMapData::DetectorSpectrumMapData(const std::string &filename) {
  if (filename.empty()) {
    throw std::runtime_error("A detector-spectrum map file must be specified.");
  }
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
    std::istringstream isstream(line);
    if (!(isstream >> m_detectors[entryNumber] >> m_spectra[entryNumber])) {
      break;
    } // error

    entryNumber++;
  }
}

void DetectorSpectrumMapData::decodeMessage(const uint8_t *buf) {
  auto messageData = GetSpectraDetectorMapping(buf);

  auto detFBVector = messageData->detector_id();
  auto specFBVector = messageData->spectrum();
  setNumberOfEntries(messageData->n_spectra());
  m_detectors.resize(static_cast<size_t>(m_numberOfEntries));
  m_spectra.resize(static_cast<size_t>(m_numberOfEntries));
  std::copy(detFBVector->begin(), detFBVector->end(), m_detectors.begin());
  std::copy(specFBVector->begin(), specFBVector->end(), m_spectra.begin());
}

Streamer::Message DetectorSpectrumMapData::getBuffer() {
  flatbuffers::FlatBufferBuilder builder;

  auto messageFlatbuf = CreateSpectraDetectorMapping(
      builder, builder.CreateVector(m_spectra),
      builder.CreateVector(m_detectors), m_numberOfEntries);
  FinishSpectraDetectorMappingBuffer(builder, messageFlatbuf);

  return Streamer::Message(builder.Release());
}

#include "DetectorSpectrumMapData.h"
#include <fstream>
#include <sstream>

DetectorSpectrumMapData::DetectorSpectrumMapData(const std::string &filename) {
  readFile(filename);
}

void DetectorSpectrumMapData::readFile(const std::string &filename) {
  std::ifstream infile(filename);
  std::string line;
  std::getline(infile, line); // discard first line
  std::getline(infile, line); // line with the number of entries
  std::istringstream iss(line);
  iss >> m_numberOfEntries;
  m_detectors.resize(m_numberOfEntries);
  m_spectra.resize(m_numberOfEntries);
  std::getline(infile, line); // discard third line
  size_t entryNumber = 0;
  while (std::getline(infile, line))
  {
    std::istringstream iss(line);
    if (!(iss >> m_detectors[entryNumber] >> m_spectra[entryNumber])) {
      break;
    } // error

    entryNumber++;
  }
}

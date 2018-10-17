#include "data/vocab.h"
#include "data/vocab_base.h"

namespace marian {

Ptr<VocabBase> createDefaultVocab();
Ptr<VocabBase> createSentencePieceVocab(const std::string& /*vocabPath*/, Ptr<Options>, size_t /*batchIndex*/);

// @TODO: make each vocab peek on type
Ptr<VocabBase> createVocab(const std::string& vocabPath, Ptr<Options> options, size_t batchIndex) {
  auto vocab = createSentencePieceVocab(vocabPath, options, batchIndex);
  return vocab ? vocab : createDefaultVocab();
}

int Vocab::loadOrCreate(const std::string& vocabPath,
                        const std::string& trainPath,
                        int max) {
  size_t size = 0;
  if(vocabPath.empty()) {
    // No vocabulary path was given, attempt to first find a vocabulary
    // for trainPath + possible suffixes. If not found attempt to create
    // as trainPath + canonical suffix.

    LOG(info,
        "No vocabulary path given; "
        "trying to find default vocabulary based on data path {}",
        trainPath);

    vImpl_ = createDefaultVocab();
    size = vImpl_->findAndLoad(trainPath, max);

    if(size == 0) {
      auto path = trainPath + vImpl_->canonicalSuffix();
      LOG(info,
          "No vocabulary path given; "
          "trying to find vocabulary based on data path {}",
          trainPath);
      vImpl_->create(path, trainPath);
      size = vImpl_->load(path, max);
    }
  } else {
    if(!filesystem::exists(vocabPath)) {
      // Vocabulary path was given, but no vocabulary present,
      // attempt to create in specified location.
      create(vocabPath, trainPath);
    }
    // Vocabulary path exists, attempting to load
    size = load(vocabPath, max);
  }
  LOG(info, "[data] Setting vocabulary size for input {} to {}", batchIndex_, size);
  return size;
}

int Vocab::load(const std::string& vocabPath, int max) {
  if(!vImpl_)
    vImpl_ = createVocab(vocabPath, options_, batchIndex_);
  return vImpl_->load(vocabPath, max);
}

void Vocab::create(const std::string& vocabPath, const std::string& trainPath) {
  if(!vImpl_)
    vImpl_ = createVocab(vocabPath, options_, batchIndex_);
  vImpl_->create(vocabPath, trainPath);
}

void Vocab::create(io::InputFileStream& trainStrm,
                   io::OutputFileStream& vocabStrm,
                   size_t maxSize) {
  if(!vImpl_)
    vImpl_ = createDefaultVocab(); // Only DefaultVocab can be built from streams
  vImpl_->create(trainStrm, vocabStrm, maxSize);
}

void Vocab::createFake() {
  if(!vImpl_)
    vImpl_ = createDefaultVocab(); // DefaultVocab is OK here
  vImpl_->createFake();
}

// string token to token id
Word Vocab::operator[](const std::string& word) const {
  return vImpl_->operator[](word);
}

// token id to string token
const std::string& Vocab::operator[](Word id) const {
  return vImpl_->operator[](id);
}

// line of text to list of token ids, can perform tokenization
Words Vocab::encode(const std::string& line,
              bool addEOS,
              bool inference) const {
  return vImpl_->encode(line, addEOS, inference);
}

// list of token ids to single line, can perform detokenization
std::string Vocab::decode(const Words& sentence,
                    bool ignoreEOS) const {
  return vImpl_->decode(sentence, ignoreEOS);
}

// number of vocabulary items
size_t Vocab::size() const { return vImpl_->size(); }

// number of vocabulary items
std::string Vocab::type() const { return vImpl_->type(); }

// return EOS symbol id
Word Vocab::getEosId() const { return vImpl_->getEosId(); }

// return UNK symbol id
Word Vocab::getUnkId() const { return vImpl_->getUnkId(); }

}  // namespace marian

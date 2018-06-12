/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

// All HloInstruction subclasses are put in this file.

#ifndef TENSORFLOW_COMPILER_XLA_SERVICE_HLO_INSTRUCTIONS_H_
#define TENSORFLOW_COMPILER_XLA_SERVICE_HLO_INSTRUCTIONS_H_

#include "tensorflow/compiler/xla/service/hlo_instruction.h"

namespace xla {

class HloBatchNormInstruction : public HloInstruction {
 public:
  // Returns feature_index field associated with the instruction. The index
  // represents the index of the feature dimension.
  int64 feature_index() const { return feature_index_; }

  // Returns a epsilon value associated with the instruction. The is a small
  // number added to the variance to avoid divide-by-zero error.
  float epsilon() const { return epsilon_; }

  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

 protected:
  explicit HloBatchNormInstruction(HloOpcode opcode, const Shape& shape,
                                   HloInstruction* operand,
                                   HloInstruction* scale, float epsilon,
                                   int64 feature_index);

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // A small float number added to the variance to avoid divide-by-zero error.
  float epsilon_ = 0.0f;

  // An integer value representing the index of the feature dimension.
  int64 feature_index_ = -1;
};

class HloBatchNormTrainingInstruction : public HloBatchNormInstruction {
 public:
  explicit HloBatchNormTrainingInstruction(const Shape& shape,
                                           HloInstruction* operand,
                                           HloInstruction* scale,
                                           HloInstruction* offset,
                                           float epsilon, int64 feature_index);

 private:
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;
};

class HloBatchNormInferenceInstruction : public HloBatchNormInstruction {
 public:
  explicit HloBatchNormInferenceInstruction(
      const Shape& shape, HloInstruction* operand, HloInstruction* scale,
      HloInstruction* offset, HloInstruction* mean, HloInstruction* variance,
      float epsilon, int64 feature_index);

 private:
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;
};

class HloBatchNormGradInstruction : public HloBatchNormInstruction {
 public:
  explicit HloBatchNormGradInstruction(
      const Shape& shape, HloInstruction* operand, HloInstruction* scale,
      HloInstruction* mean, HloInstruction* variance,
      HloInstruction* grad_output, float epsilon, int64 feature_index);

 private:
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;
};

class HloFftInstruction : public HloInstruction {
 public:
  explicit HloFftInstruction(const Shape& shape, HloInstruction* operand,
                             FftType fft_type,
                             tensorflow::gtl::ArraySlice<int64> fft_length);
  FftType fft_type() const { return fft_type_; }

  const std::vector<int64>& fft_length() const { return fft_length_; }

  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;

  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;

  // Describes FFT type for an FFT instruction.
  FftType fft_type_ = FftType::FFT;

  // Indicates the FFT length for an FFT instruction.
  std::vector<int64> fft_length_;
};

class HloSendRecvInstruction : public HloInstruction {
 public:
  // Returns the channel id associated with the instruction. The id is
  // shared between each Send/Recv pair and is globally unique to identify each
  // channel.
  int64 channel_id() const { return channel_id_; }

  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

 protected:
  explicit HloSendRecvInstruction(HloOpcode opcode, const Shape& shape,
                                  int64 channel_id);

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // Represents a unique identifier for each Send/Recv instruction pair.
  int64 channel_id_;
};

class HloSendInstruction : public HloSendRecvInstruction {
 public:
  explicit HloSendInstruction(HloInstruction* operand, int64 channel_id);

 private:
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;
};

class HloSendDoneInstruction : public HloSendRecvInstruction {
 public:
  explicit HloSendDoneInstruction(HloSendInstruction* operand);

 private:
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;
};

class HloRecvInstruction : public HloSendRecvInstruction {
 public:
  explicit HloRecvInstruction(const Shape& shape, int64 channel_id);

 private:
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;
};

class HloRecvDoneInstruction : public HloSendRecvInstruction {
 public:
  explicit HloRecvDoneInstruction(HloRecvInstruction* operand);

 private:
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;
};

class HloReverseInstruction : public HloInstruction {
 public:
  explicit HloReverseInstruction(const Shape& shape, HloInstruction* operand,
                                 tensorflow::gtl::ArraySlice<int64> dimensions);
  // Returns the dimension sizes or numbers associated with this instruction.
  const std::vector<int64>& dimensions() const override { return dimensions_; }
  int64 dimensions(int64 index) const override { return dimensions()[index]; }
  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;

  std::vector<int64> dimensions_;
};

class HloConcatenateInstruction : public HloInstruction {
 public:
  explicit HloConcatenateInstruction(
      const Shape& shape, tensorflow::gtl::ArraySlice<HloInstruction*> operands,
      int64 dimension);
  // Returns the dimension sizes or numbers associated with this instruction.
  const std::vector<int64>& dimensions() const override { return dimensions_; }
  int64 dimensions(int64 index) const override { return dimensions()[index]; }
  // Accessor for the dimension in which a concatenate HLO should occur.
  int64 concatenate_dimension() const { return dimensions(0); }
  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;

  std::vector<int64> dimensions_;
};

class HloReduceInstruction : public HloInstruction {
 public:
  explicit HloReduceInstruction(
      const Shape& shape, HloInstruction* arg, HloInstruction* init_value,
      tensorflow::gtl::ArraySlice<int64> dimensions_to_reduce,
      HloComputation* reduce_computation);
  // Returns the dimension sizes or numbers associated with this instruction.
  const std::vector<int64>& dimensions() const override { return dimensions_; }
  int64 dimensions(int64 index) const override { return dimensions()[index]; }
  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;

  std::vector<int64> dimensions_;
};

class HloTransposeInstruction : public HloInstruction {
 public:
  explicit HloTransposeInstruction(
      const Shape& shape, HloInstruction* operand,
      tensorflow::gtl::ArraySlice<int64> dimensions);
  // Returns the dimension sizes or numbers associated with this instruction.
  const std::vector<int64>& dimensions() const override { return dimensions_; }
  int64 dimensions(int64 index) const override { return dimensions()[index]; }
  // Returns whether this instruction does a rank-2 transposition.
  bool IsRank2Transpose() const;
  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;

  std::vector<int64> dimensions_;
};

class HloBroadcastInstruction : public HloInstruction {
 public:
  explicit HloBroadcastInstruction(
      const Shape& shape, HloInstruction* operand,
      tensorflow::gtl::ArraySlice<int64> broadcast_dimension);
  // Returns the dimension sizes or numbers associated with this instruction.
  const std::vector<int64>& dimensions() const override { return dimensions_; }
  int64 dimensions(int64 index) const override { return dimensions()[index]; }
  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;

  std::vector<int64> dimensions_;
};

class HloMapInstruction : public HloInstruction {
 public:
  explicit HloMapInstruction(
      const Shape& shape, tensorflow::gtl::ArraySlice<HloInstruction*> operands,
      HloComputation* map_computation,
      tensorflow::gtl::ArraySlice<HloInstruction*> static_operands = {});
  // Returns the dimension sizes or numbers associated with this instruction.
  const std::vector<int64>& dimensions() const override { return dimensions_; }
  int64 dimensions(int64 index) const override { return dimensions()[index]; }
  // Returns a serialized representation of this instruction.
  HloInstructionProto ToProto() const override;

  // Returns true if this instruction is binary and elementwise.
  bool IsElementwise() const override;

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;

  std::vector<int64> dimensions_;
};

class HloSliceInstruction : public HloInstruction {
 public:
  explicit HloSliceInstruction(const Shape& shape, HloInstruction* operand,
                               tensorflow::gtl::ArraySlice<int64> start_indices,
                               tensorflow::gtl::ArraySlice<int64> limit_indices,
                               tensorflow::gtl::ArraySlice<int64> strides);

  HloInstructionProto ToProto() const override;

  // Returns the start index in the given dimension for a slice node.
  int64 slice_starts(int64 dimension) const { return slice_starts_[dimension]; }
  const std::vector<int64>& slice_starts() const { return slice_starts_; }

  // Returns the (exclusive) limit index in the given dimension for a slice
  // node.
  int64 slice_limits(int64 dimension) const { return slice_limits_[dimension]; }
  const std::vector<int64>& slice_limits() const { return slice_limits_; }

  // Returns the stride in the given dimension for a slice node.
  int64 slice_strides(int64 dimension) const {
    return slice_strides_[dimension];
  }
  const std::vector<int64>& slice_strides() const { return slice_strides_; }

  // Returns the flag that describes whether a slice must be lowered into an
  // offset into the original operand.
  bool IsInPlaceSlice() const { return is_in_place_slice_; }

  // Sets and returns the flag that describes whether a slice must be lowered
  // into an offset into the original operand.
  bool SetIsInPlaceSlice(bool value) {
    is_in_place_slice_ = value;
    return value;
  }

 private:
  std::vector<string> ExtraAttributesToStringImpl(
      const HloPrintOptions& options) const override;
  bool IdenticalSlowPath(
      const HloInstruction& other,
      const std::function<bool(const HloComputation*, const HloComputation*)>&
          eq_computations) const override;
  // Implementation for non-common logic of CloneWithNewOperands.
  std::unique_ptr<HloInstruction> CloneWithNewOperandsImpl(
      const Shape& shape,
      tensorflow::gtl::ArraySlice<HloInstruction*> new_operands,
      HloCloneContext* context) const override;

  // Describes the [begin, end) index range for a slice.
  std::vector<int64> slice_starts_;
  std::vector<int64> slice_limits_;
  std::vector<int64> slice_strides_;

  // Describes whether the slice can be lowered to an offset into the operand.
  bool is_in_place_slice_ = false;
};
}  // namespace xla

#endif  // TENSORFLOW_COMPILER_XLA_SERVICE_HLO_INSTRUCTIONS_H_
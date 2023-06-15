/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_federated/task/model.h"
#include <cstddef>

#include "brave/components/brave_federated/util/linear_algebra_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/googletest/src/googletest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BraveFederatedLearning*

using testing::ElementsAreArray;

namespace brave_federated {

ModelSpec kModelSpec = {
    64,   // num_params
    32,   // batch_size
    0.1,  // learning_rate
    10,   // num_iterations
    0.7   // threshold
};

TEST(BraveFederatedLearningModelTest, GetModelSize) {
  // Arrange
  Model model(kModelSpec);

  // Act
  size_t model_size = model.GetModelSize();

  // Assert
  EXPECT_EQ(static_cast<int>(model_size), kModelSpec.num_params);
}

TEST(BraveFederatedLearningModelTest, GetModelSizeAfterSettingWeights) {
  // Arrange
  Model model(kModelSpec);
  Weights weights = {1.0, 2.0, 3.0, 4.0, 5.0};

  // Act
  model.SetWeights(weights);
  size_t model_size = model.GetModelSize();

  // Assert
  EXPECT_EQ(model_size, weights.size());
}

TEST(BraveFederatedLearningModelTest, GetBatchSize) {
  // Arrange
  Model model(kModelSpec);
  Weights weights = {1.0, 2.0, 3.0, 4.0, 5.0};

  // Act

  // Assert
  EXPECT_EQ(static_cast<int>(model.GetBatchSize()), kModelSpec.batch_size);
}

TEST(BraveFederatedLearningModelTest, Train) {
  // Arrange
  DataSet test_data = {
      {1.1, 2.2, 3.3, 4.4, 5.5},
      {1.2, 3.2, 3.4, 5.4, 6.5},
      {5.0, 4.0, 3.0, 2.0, 1.0},
      {1.0, 1.0, 1.0, 1.0, 1.0},
  };
  ModelSpec model_spec = {
      static_cast<int>(test_data[0].size()),  // num_params
      2,                                      // batch_size
      0.1,                                    // learning_rate
      1,                                      // num_iterations
      0.7                                     // threshold
  };
  Model model(model_spec);

  // Act
  PerformanceReport train_report = model.Train(test_data);

  // Assert
  EXPECT_EQ(train_report.dataset_size, test_data.size());
  EXPECT_EQ(static_cast<int>(train_report.parameters[0].size()),
            model_spec.num_params);
  EXPECT_EQ(train_report.parameters[1].size(), 1U);
}

TEST(BraveFederatedLearningModelTest, TrainOnEmptyDataset) {
  // Arrange
  DataSet test_data = {};
  ModelSpec model_spec = {
      5,    // num_params
      2,    // batch_size
      0.1,  // learning_rate
      1,    // num_iterations
      0.7   // threshold
  };
  Model model(model_spec);

  // Act
  Weights initial_weights = model.GetWeights();
  float initial_bias = model.GetBias();
  PerformanceReport train_report = model.Train(test_data);

  // Assert
  EXPECT_EQ(train_report.dataset_size, test_data.size());
  EXPECT_THAT(train_report.parameters.at(0), ElementsAreArray(initial_weights));
  EXPECT_EQ(train_report.parameters.at(1).at(0), initial_bias);
  EXPECT_EQ(train_report.accuracy, 0.0);
  EXPECT_EQ(train_report.loss, 0.0);
}

TEST(BraveFederatedLearningModelTest, Evaluate) {
  // Arrange
  DataSet test_data = {
      {1.1, 2.2, 3.3, 4.4, 5.5},
      {1.2, 3.2, 3.4, 5.4, 6.5},
      {5.0, 4.0, 3.0, 2.0, 1.0},
      {1.0, 1.0, 1.0, 1.0, 1.0},
  };
  ModelSpec model_spec = {
      static_cast<int>(test_data[0].size()),  // num_params
      2,                                      // batch_size
      0.1,                                    // learning_rate
      1,                                      // num_iterations
      0.7                                     // threshold
  };
  Model model(model_spec);

  // Act
  PerformanceReport evaluate_report = model.Evaluate(test_data);

  // Assert
  EXPECT_EQ(evaluate_report.dataset_size, test_data.size());
  EXPECT_EQ(evaluate_report.parameters.size(), 0U);
}

TEST(BraveFederatedLearningModelTest, EvaluateWithEmptyDataset) {
  // Arrange
  DataSet test_data = {};
  ModelSpec model_spec = {
      5,    // num_params
      2,    // batch_size
      0.1,  // learning_rate
      1,    // num_iterations
      0.7   // threshold
  };
  Model model(model_spec);

  // Act
  PerformanceReport evaluate_report = model.Evaluate(test_data);

  // Assert
  EXPECT_EQ(evaluate_report.dataset_size, 0U);
  EXPECT_EQ(evaluate_report.parameters.size(), 0U);
  EXPECT_EQ(evaluate_report.accuracy, 0.0);
  EXPECT_EQ(evaluate_report.loss, 0.0);
}

TEST(BraveFederatedLearningModelTest, Predict) {
  // Arrange
  DataSet test_data = {
      {1.1, 2.2, 3.3, 4.4, 5.5},
      {1.2, 3.2, 3.4, 5.4, 6.5},
      {5.0, 4.0, 3.0, 2.0, 1.0},
      {1.0, 1.0, 1.0, 1.0, 1.0},
  };
  ModelSpec model_spec = {
      static_cast<int>(test_data[0].size()),  // num_params
      2,                                      // batch_size
      0.1,                                    // learning_rate
      1,                                      // num_iterations
      0.7                                     // threshold
  };
  Model model(model_spec);

  // Act
  std::vector<float> predictions = model.Predict(test_data);

  // Assert
  EXPECT_EQ(predictions.size(), test_data.size());
}

TEST(BraveFederatedLearningModelTest, PredictWithEmptyDataset) {
  // Arrange
  DataSet test_data = {};
  ModelSpec model_spec = {
      5,    // num_params
      2,    // batch_size
      0.1,  // learning_rate
      1,    // num_iterations
      0.7   // threshold
  };
  Model model(model_spec);

  // Act
  std::vector<float> predictions = model.Predict(test_data);

  // Assert
  EXPECT_EQ(predictions.size(), 0U);
}

}  // namespace brave_federated

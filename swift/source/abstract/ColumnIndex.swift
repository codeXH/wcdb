/*
 * Tencent is pleased to support the open source community by making
 * WCDB available.
 *
 * Copyright (C) 2017 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import Foundation

public final class ColumnIndex: Describable {
    public private(set) var description: String

    public init(with columnConvertible: ColumnConvertible, orderBy term: OrderTerm? = nil) {
        description = columnConvertible.asColumn().description
        if let wrappedTerm = term {
            description.append(" \(wrappedTerm.description)")
        }
    }

    public init(with expressionConvertible: ExpressionConvertible, orderBy term: OrderTerm? = nil) {
        description = expressionConvertible.asExpression().description
        if let wrappedTerm = term {
            description = " \(wrappedTerm.description)"
        }
    }
}

extension ColumnIndex: ColumnIndexConvertible {
    public func asIndex() -> ColumnIndex {
        return self
    }
}

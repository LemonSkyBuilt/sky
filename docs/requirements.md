# 证券行情、模拟撮合与组合风控平台需求文档

## 1. 文档信息

- 项目名称：证券行情、模拟撮合与组合风控平台
- 文档版本：v0.1
- 文档日期：2026-03-25
- 文档状态：初稿
- 目标岗位：社招后端工程师
- 技术方向：`Java + C++` 混合架构、金融后端、事件驱动、风控与撮合

## 2. 项目背景

本项目面向研究员、交易员和风控人员，提供证券行情接入、模拟下单、撮合成交、账户持仓更新、组合风险分析、回测分析和审计监控能力。

项目目标不是做一个普通股票信息网站或 CRUD 管理平台，而是构建一个接近真实金融后端系统的项目，用于体现以下能力：

- 企业级后端服务设计与实现
- 高性能核心链路设计
- 事件驱动架构设计
- 金融交易与风控业务理解
- 可观测性、幂等性、可恢复性等工程能力

## 3. 建设目标

### 3.1 总体目标

构建一个基于 `Java + C++` 的证券交易仿真与风险分析平台，完成从行情接入到订单流转、撮合成交、账户更新、持仓变更、风险计算、回测分析的全链路闭环。

### 3.2 简历目标

项目应能支撑以下简历表达：

- 有完整的系统边界和服务拆分
- 有接近真实生产问题的技术设计
- 能解释为什么混合使用 `Java` 与 `C++`
- 能展示后端工程能力，而非简单页面与接口堆砌

### 3.3 非目标

以下能力不纳入第一阶段目标：

- 真实券商柜台接入
- 实盘交易
- 复杂衍生品交易
- 多租户 SaaS 化改造
- 超大规模分布式部署

## 4. 项目亮点

- `Java` 负责订单、账户、组合、回测、审计等中后台服务
- `C++` 负责模拟撮合引擎、行情回放引擎、实时风险计算核心
- 基于 `Kafka` 实现订单、成交、风险事件的异步解耦
- 基于 `gRPC + Protobuf` 实现 Java 与 C++ 服务通信
- 使用 `PostgreSQL` 保存事务数据，`ClickHouse` 保存历史行情与分析数据
- 使用 `Redis` 提供热点行情缓存与账户快照缓存
- 内置幂等消费、失败重试、数据重放、审计日志与监控告警能力

## 5. 系统架构

### 5.1 总体架构

```text
                  +----------------------+
                  |  Market Data Adapter |
                  +----------+-----------+
                             |
                             v
                           Kafka
                             |
            +----------------+----------------+
            |                                 |
            v                                 v
+-------------------------+      +---------------------------+
| Java market-service     |      | C++ market-replay-engine |
| 清洗、去重、缓存、查询   |      | 历史行情回放              |
+-----------+-------------+      +-------------+-------------+
            |                                      |
            v                                      v
 ClickHouse / Redis                      +--------------------+
                                         | C++ matching-engine|
                                         | 订单簿 / 撮合       |
                                         +---------+----------+
                                                   |
                                                   v
                                                 Kafka
                                                   |
                 +----------------+----------------+----------------+
                 |                |                                 |
                 v                v                                 v
      +----------------+  +-------------------+         +------------------+
      | order-service  |  | account-service   |         | portfolio-service|
      | 订单状态管理    |  | 资金、流水、冻结  |         | 持仓、收益、快照 |
      +----------------+  +-------------------+         +------------------+
                                                   |
                                                   v
                                           +------------------+
                                           | C++ risk-core    |
                                           | 实时风险计算      |
                                           +--------+---------+
                                                    |
                                                    v
                                             Kafka / gRPC
                                                    |
                                                    v
                                           +------------------+
                                           | Java risk-service|
                                           | 风险事件、告警    |
                                           +------------------+
```

### 5.2 架构原则

- 在线事务链路与历史分析链路分离
- Java 负责业务编排，C++ 负责低延迟核心
- 核心状态变化通过事件流驱动
- 数据链路支持重放、补数和失败恢复
- 所有关键动作保留审计信息

## 6. 语言与模块分工

### 6.1 Java 模块职责

- 用户、账户、权限
- 订单管理 API
- 组合管理
- 回测任务调度
- 风控配置中心
- 行情查询服务
- 报表与审计
- 消息队列消费、任务编排、监控接入

### 6.2 C++ 模块职责

- 模拟撮合引擎
- 行情回放引擎
- 实时风险计算核心
- 低延迟订单簿处理
- 高频场景下的高性能计算模块

### 6.3 设计原则

- 不使用 `JNI` 直接嵌入 Java 进程
- 通过独立 `C++` 服务与 `gRPC` 做跨语言通信
- Java 服务强调可维护性和业务表达能力
- C++ 服务强调延迟、吞吐和资源控制能力

## 7. 核心业务流程

### 7.1 行情接入流程

1. 行情采集服务从外部数据源拉取日线、分钟线或历史数据。
2. 行情事件写入 `Kafka`。
3. `market-service` 消费事件，完成清洗、去重、格式统一、缓存刷新。
4. 清洗后的行情数据落库到 `ClickHouse`。
5. 热点股票快照写入 `Redis`。

### 7.2 订单撮合流程

1. 用户通过 `order-service` 提交模拟买卖委托。
2. 订单写入事务库并通过 outbox 或可靠消息机制投递到 `Kafka`。
3. `C++ matching-engine` 消费订单事件，维护订单簿并执行价格优先、时间优先撮合。
4. 产生的成交回报写入 `Kafka`。
5. `order-service` 更新订单状态。
6. `account-service` 与 `portfolio-service` 更新资金、持仓和收益。

### 7.3 实时风控流程

1. `C++ risk-core` 订阅行情与持仓变化。
2. 计算最大回撤、仓位占比、单票集中度、止损阈值等指标。
3. 风险触发后写入风险事件流。
4. `risk-service` 持久化风险事件并触发告警。

### 7.4 回测流程

1. 用户通过 `backtest-service` 提交回测任务。
2. 任务进入异步执行队列。
3. `market-replay-engine` 执行历史行情回放。
4. 回测结果写入结果表，支持查询收益率、回撤、夏普等指标。

## 8. 服务拆分

| 服务名 | 技术栈 | 主要职责 |
| --- | --- | --- |
| `gateway-service` | Java | 网关、鉴权、限流、路由 |
| `market-service` | Java | 行情接入、清洗、缓存、查询 |
| `order-service` | Java | 下单、撤单、订单状态流转、幂等校验 |
| `account-service` | Java | 资金账户、流水、冻结资金、余额更新 |
| `portfolio-service` | Java | 持仓、成本、组合收益、快照 |
| `risk-service` | Java | 风控规则配置、风险事件持久化、风险查询 |
| `backtest-service` | Java | 回测任务编排、参数管理、结果查询 |
| `audit-service` | Java | 审计日志、操作追踪、导出记录 |
| `matching-engine` | C++ | 模拟撮合、订单簿、成交回报 |
| `risk-core` | C++ | 实时风险指标计算 |
| `market-replay-engine` | C++ | 历史行情回放、回测驱动 |

## 9. 技术选型

| 领域 | 选型 |
| --- | --- |
| 主后端 | `Java 21 + Spring Boot` |
| 高性能核心 | `C++17/20` |
| 跨语言通信 | `gRPC + Protobuf` |
| 消息队列 | `Kafka` |
| 缓存 | `Redis` |
| 事务数据库 | `PostgreSQL` |
| 历史分析库 | `ClickHouse` |
| 监控 | `Prometheus + Grafana` |
| 日志 | `Loki` 或 `ELK` |
| 部署 | `Docker Compose` |
| 任务调度 | 可选 `XXL-JOB` |

### 9.1 Java 开发环境约定

- Java 部分默认使用 `IntelliJ IDEA` 开发与调试，优先推荐 `IntelliJ IDEA Ultimate`
- 如果使用免费版本，可使用 `IntelliJ IDEA Community` 完成第一阶段开发
- Java 版本统一为 `JDK 21 (LTS)`，推荐发行版为 `Temurin 21`
- Java 服务构建工具统一使用 `Maven`，采用多模块工程方式组织
- 第一阶段不引入 `Gradle`，避免双构建体系增加维护成本
- `C++` 模块不要求在 `IDEA` 中开发，建议使用 `CLion` 或 `VS Code + CMake`
## 10. 功能需求

### 10.1 行情模块

- 支持导入日线和分钟线数据
- 支持行情清洗、去重、落库
- 支持股票快照查询
- 支持历史 K 线查询
- 支持热点行情缓存

### 10.2 订单模块

- 支持限价买入、限价卖出
- 支持撤单
- 支持订单详情查询
- 支持订单状态流转
- 支持按用户维度分页查询订单

### 10.3 撮合模块

- 支持订单簿维护
- 支持价格优先、时间优先撮合
- 支持部分成交
- 支持撤单处理
- 支持成交回报输出

### 10.4 账户模块

- 支持账户余额管理
- 支持冻结资金与可用资金管理
- 支持资金流水记录
- 支持成交后的资金变更

### 10.5 持仓与组合模块

- 支持持仓数量、可卖数量、冻结数量维护
- 支持均价成本计算
- 支持浮盈亏、已实现盈亏计算
- 支持每日组合快照生成

### 10.6 风控模块

- 支持配置风控规则
- 支持最大回撤计算
- 支持仓位占比校验
- 支持单票集中度校验
- 支持止损阈值触发
- 支持风险事件查询

### 10.7 回测模块

- 支持回测任务创建
- 支持异步回测执行
- 支持回测任务状态查询
- 支持输出收益率、年化收益、最大回撤、夏普、胜率、交易次数

### 10.8 审计模块

- 记录下单、撤单、回测、规则修改、导出报表等动作
- 保留请求参数、结果状态、操作人和时间

## 11. 非功能需求

### 11.1 幂等性

- 下单接口通过 `X-Request-Id` 或 `request_id` 实现幂等
- 成交事件通过 `trade_id` 去重消费
- 回测任务通过 `job_id` 保证唯一性

### 11.2 一致性

- 下单与消息投递采用 outbox 或等价机制
- 成交驱动账户和持仓更新，保证链路可重放

### 11.3 可恢复性

- 支持行情补数与重放
- 支持订单事件重放
- 支持死信和失败重试

### 11.4 可观测性

- 暴露订单耗时、撮合耗时、回测耗时、Kafka Lag、风险计算耗时等指标
- 支持应用日志集中采集

### 11.5 安全性

- 至少区分 `trader` 和 `risk_admin` 两类角色
- 所有写操作具备审计追踪能力

## 12. 订单状态机

```text
NEW -> ACCEPTED -> PARTIALLY_FILLED -> FILLED
NEW -> ACCEPTED -> CANCELED
NEW -> REJECTED
PARTIALLY_FILLED -> CANCELED
```

## 13. 项目目录规划

```text
trading-platform/
  pom.xml
  README.md
  docs/
    requirements.md
    architecture.md
    api.md
    interview-notes.md
    benchmark.md
  deploy/
    docker-compose.yml
    prometheus.yml
    grafana/
  proto/
    trading.proto
    risk.proto
  java-services/
    pom.xml
    gateway-service/
    market-service/
    order-service/
    account-service/
    portfolio-service/
    risk-service/
    backtest-service/
    audit-service/
    common-starter/
  cpp-services/
    matching-engine/
    risk-core/
    market-replay-engine/
  sql/
    postgres/
    clickhouse/
  scripts/
    load-market-data/
    mock-order-flow/
    benchmark/
```

## 14. 数据库设计

### 14.1 PostgreSQL 表

| 表名 | 用途 |
| --- | --- |
| `user_account` | 用户信息、角色、状态 |
| `cash_account` | 账户余额、冻结资金、可用资金 |
| `stock_order` | 订单主表 |
| `stock_trade` | 成交主表 |
| `position` | 持仓信息 |
| `account_flow` | 资金流水 |
| `portfolio_snapshot` | 每日组合快照 |
| `risk_rule` | 风控规则配置 |
| `risk_event` | 风险事件 |
| `backtest_job` | 回测任务 |
| `backtest_result` | 回测结果 |
| `audit_log` | 审计日志 |
| `event_outbox` | 可靠消息表 |

### 14.2 ClickHouse 表

| 表名 | 用途 |
| --- | --- |
| `market_bar_1d` | 股票日线行情 |
| `market_bar_1m` | 股票分钟线行情 |

### 14.3 PostgreSQL 初始化 SQL 初稿

```sql
CREATE TABLE user_account (
    id BIGSERIAL PRIMARY KEY,
    user_code VARCHAR(64) NOT NULL UNIQUE,
    user_name VARCHAR(128) NOT NULL,
    role_code VARCHAR(32) NOT NULL,
    risk_level VARCHAR(16) NOT NULL DEFAULT 'NORMAL',
    status VARCHAR(16) NOT NULL DEFAULT 'ACTIVE',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE cash_account (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES user_account(id),
    currency_code VARCHAR(8) NOT NULL DEFAULT 'CNY',
    total_balance NUMERIC(18, 2) NOT NULL DEFAULT 0,
    available_balance NUMERIC(18, 2) NOT NULL DEFAULT 0,
    frozen_balance NUMERIC(18, 2) NOT NULL DEFAULT 0,
    version BIGINT NOT NULL DEFAULT 0,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE (user_id, currency_code)
);

CREATE TABLE stock_order (
    id BIGSERIAL PRIMARY KEY,
    order_id VARCHAR(64) NOT NULL UNIQUE,
    request_id VARCHAR(64) NOT NULL,
    client_order_id VARCHAR(64),
    user_id BIGINT NOT NULL REFERENCES user_account(id),
    symbol VARCHAR(32) NOT NULL,
    side VARCHAR(8) NOT NULL,
    order_type VARCHAR(16) NOT NULL,
    price NUMERIC(18, 4),
    quantity BIGINT NOT NULL,
    filled_quantity BIGINT NOT NULL DEFAULT 0,
    avg_fill_price NUMERIC(18, 4) NOT NULL DEFAULT 0,
    status VARCHAR(32) NOT NULL,
    source VARCHAR(32) NOT NULL DEFAULT 'WEB',
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE (user_id, request_id)
);

CREATE INDEX idx_stock_order_user_created
    ON stock_order(user_id, created_at DESC);

CREATE INDEX idx_stock_order_symbol_status
    ON stock_order(symbol, status);

CREATE TABLE stock_trade (
    id BIGSERIAL PRIMARY KEY,
    trade_id VARCHAR(64) NOT NULL UNIQUE,
    order_id VARCHAR(64) NOT NULL REFERENCES stock_order(order_id),
    user_id BIGINT NOT NULL REFERENCES user_account(id),
    symbol VARCHAR(32) NOT NULL,
    side VARCHAR(8) NOT NULL,
    price NUMERIC(18, 4) NOT NULL,
    quantity BIGINT NOT NULL,
    trade_amount NUMERIC(18, 4) NOT NULL,
    traded_at TIMESTAMPTZ NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_stock_trade_user_traded
    ON stock_trade(user_id, traded_at DESC);

CREATE INDEX idx_stock_trade_symbol_traded
    ON stock_trade(symbol, traded_at DESC);

CREATE TABLE position (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES user_account(id),
    symbol VARCHAR(32) NOT NULL,
    total_quantity BIGINT NOT NULL DEFAULT 0,
    sellable_quantity BIGINT NOT NULL DEFAULT 0,
    frozen_quantity BIGINT NOT NULL DEFAULT 0,
    avg_cost NUMERIC(18, 4) NOT NULL DEFAULT 0,
    last_price NUMERIC(18, 4) NOT NULL DEFAULT 0,
    market_value NUMERIC(18, 4) NOT NULL DEFAULT 0,
    unrealized_pnl NUMERIC(18, 4) NOT NULL DEFAULT 0,
    realized_pnl NUMERIC(18, 4) NOT NULL DEFAULT 0,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE (user_id, symbol)
);

CREATE TABLE account_flow (
    id BIGSERIAL PRIMARY KEY,
    flow_id VARCHAR(64) NOT NULL UNIQUE,
    user_id BIGINT NOT NULL REFERENCES user_account(id),
    currency_code VARCHAR(8) NOT NULL DEFAULT 'CNY',
    flow_type VARCHAR(32) NOT NULL,
    amount NUMERIC(18, 2) NOT NULL,
    balance_before NUMERIC(18, 2) NOT NULL,
    balance_after NUMERIC(18, 2) NOT NULL,
    ref_type VARCHAR(32),
    ref_id VARCHAR(64),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_account_flow_user_created
    ON account_flow(user_id, created_at DESC);

CREATE TABLE portfolio_snapshot (
    id BIGSERIAL PRIMARY KEY,
    snapshot_date DATE NOT NULL,
    user_id BIGINT NOT NULL REFERENCES user_account(id),
    total_asset NUMERIC(18, 2) NOT NULL,
    cash_asset NUMERIC(18, 2) NOT NULL,
    position_asset NUMERIC(18, 2) NOT NULL,
    daily_pnl NUMERIC(18, 2) NOT NULL DEFAULT 0,
    total_pnl NUMERIC(18, 2) NOT NULL DEFAULT 0,
    daily_return NUMERIC(12, 6) NOT NULL DEFAULT 0,
    total_return NUMERIC(12, 6) NOT NULL DEFAULT 0,
    max_drawdown NUMERIC(12, 6) NOT NULL DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE (snapshot_date, user_id)
);

CREATE TABLE risk_rule (
    id BIGSERIAL PRIMARY KEY,
    rule_code VARCHAR(64) NOT NULL UNIQUE,
    rule_name VARCHAR(128) NOT NULL,
    rule_type VARCHAR(32) NOT NULL,
    threshold_value NUMERIC(18, 6) NOT NULL,
    scope_type VARCHAR(32) NOT NULL,
    scope_value VARCHAR(64),
    status VARCHAR(16) NOT NULL DEFAULT 'ENABLED',
    created_by VARCHAR(64) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE risk_event (
    id BIGSERIAL PRIMARY KEY,
    event_id VARCHAR(64) NOT NULL UNIQUE,
    user_id BIGINT REFERENCES user_account(id),
    symbol VARCHAR(32),
    rule_code VARCHAR(64) NOT NULL,
    risk_type VARCHAR(32) NOT NULL,
    trigger_value NUMERIC(18, 6) NOT NULL,
    threshold_value NUMERIC(18, 6) NOT NULL,
    severity VARCHAR(16) NOT NULL,
    status VARCHAR(16) NOT NULL DEFAULT 'OPEN',
    occurred_at TIMESTAMPTZ NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_risk_event_user_occurred
    ON risk_event(user_id, occurred_at DESC);

CREATE TABLE backtest_job (
    id BIGSERIAL PRIMARY KEY,
    job_id VARCHAR(64) NOT NULL UNIQUE,
    strategy_name VARCHAR(128) NOT NULL,
    symbol VARCHAR(32) NOT NULL,
    start_date DATE NOT NULL,
    end_date DATE NOT NULL,
    params_json JSONB NOT NULL,
    status VARCHAR(16) NOT NULL,
    requested_by VARCHAR(64) NOT NULL,
    submitted_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    started_at TIMESTAMPTZ,
    finished_at TIMESTAMPTZ
);

CREATE TABLE backtest_result (
    id BIGSERIAL PRIMARY KEY,
    job_id VARCHAR(64) NOT NULL UNIQUE REFERENCES backtest_job(job_id),
    total_return NUMERIC(12, 6) NOT NULL DEFAULT 0,
    annual_return NUMERIC(12, 6) NOT NULL DEFAULT 0,
    max_drawdown NUMERIC(12, 6) NOT NULL DEFAULT 0,
    sharpe_ratio NUMERIC(12, 6) NOT NULL DEFAULT 0,
    win_rate NUMERIC(12, 6) NOT NULL DEFAULT 0,
    trade_count BIGINT NOT NULL DEFAULT 0,
    result_json JSONB NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE audit_log (
    id BIGSERIAL PRIMARY KEY,
    trace_id VARCHAR(64),
    operator_id VARCHAR(64) NOT NULL,
    action VARCHAR(64) NOT NULL,
    resource_type VARCHAR(64) NOT NULL,
    resource_id VARCHAR(64),
    request_payload JSONB,
    response_code VARCHAR(32),
    result_status VARCHAR(16) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_audit_log_operator_created
    ON audit_log(operator_id, created_at DESC);

CREATE TABLE event_outbox (
    id BIGSERIAL PRIMARY KEY,
    event_id VARCHAR(64) NOT NULL UNIQUE,
    topic VARCHAR(128) NOT NULL,
    event_key VARCHAR(128) NOT NULL,
    payload JSONB NOT NULL,
    publish_status VARCHAR(16) NOT NULL DEFAULT 'NEW',
    retry_count INT NOT NULL DEFAULT 0,
    next_retry_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

### 14.4 ClickHouse 初始化 SQL 初稿

```sql
CREATE TABLE market_bar_1d
(
    trade_date Date,
    symbol String,
    open Decimal(18, 4),
    high Decimal(18, 4),
    low Decimal(18, 4),
    close Decimal(18, 4),
    volume UInt64,
    amount Decimal(18, 4),
    source String,
    ingested_at DateTime DEFAULT now()
)
ENGINE = ReplacingMergeTree(ingested_at)
PARTITION BY toYYYYMM(trade_date)
ORDER BY (symbol, trade_date);

CREATE TABLE market_bar_1m
(
    bar_time DateTime,
    trade_date Date,
    symbol String,
    open Decimal(18, 4),
    high Decimal(18, 4),
    low Decimal(18, 4),
    close Decimal(18, 4),
    volume UInt64,
    amount Decimal(18, 4),
    source String,
    ingested_at DateTime DEFAULT now()
)
ENGINE = ReplacingMergeTree(ingested_at)
PARTITION BY toYYYYMM(trade_date)
ORDER BY (symbol, bar_time);
```

## 15. API 设计模板

### 15.1 通用约定

- Base URL：`/api`
- 认证方式：`Authorization: Bearer <token>`
- 幂等头：`X-Request-Id: <uuid>`

统一响应格式：

```json
{
  "code": "0",
  "message": "OK",
  "requestId": "7b6f5c5d-1d8e-4f8b-8e98-2fd1ac0f1001",
  "data": {}
}
```

错误码示例：

- `ORDER_NOT_FOUND`
- `INSUFFICIENT_BALANCE`
- `POSITION_NOT_ENOUGH`
- `RISK_RULE_BLOCKED`
- `BACKTEST_JOB_NOT_FOUND`
- `INVALID_ARGUMENT`

### 15.2 核心接口清单

| 模块 | 方法 | 路径 | 说明 |
| --- | --- | --- | --- |
| 订单 | `POST` | `/api/orders` | 提交订单 |
| 订单 | `POST` | `/api/orders/{orderId}/cancel` | 撤单 |
| 订单 | `GET` | `/api/orders/{orderId}` | 查询订单详情 |
| 账户 | `GET` | `/api/accounts/{userId}/summary` | 查询账户摘要 |
| 持仓 | `GET` | `/api/positions?userId=...` | 查询持仓 |
| 行情 | `GET` | `/api/market/stocks/{symbol}/snapshot` | 查询股票快照 |
| 行情 | `GET` | `/api/market/stocks/{symbol}/bars` | 查询 K 线 |
| 回测 | `POST` | `/api/backtests` | 创建回测任务 |
| 回测 | `GET` | `/api/backtests/{jobId}` | 查询回测结果 |
| 风控 | `GET` | `/api/risk/users/{userId}/events` | 查询风险事件 |

### 15.3 提交订单示例

请求：

```http
POST /api/orders
Authorization: Bearer <token>
X-Request-Id: 3a7703f7-759c-4d80-98f2-a3c95a4cb2b1
Content-Type: application/json
```

```json
{
  "clientOrderId": "cli-20260325-0001",
  "userId": 10001,
  "symbol": "600519.SH",
  "side": "BUY",
  "orderType": "LIMIT",
  "price": 1688.88,
  "quantity": 100
}
```

响应：

```json
{
  "code": "0",
  "message": "OK",
  "requestId": "3a7703f7-759c-4d80-98f2-a3c95a4cb2b1",
  "data": {
    "orderId": "ord-20260325-000001",
    "status": "NEW",
    "createdAt": "2026-03-25T10:20:30+08:00"
  }
}
```

### 15.4 查询账户摘要示例

请求：

```http
GET /api/accounts/10001/summary
Authorization: Bearer <token>
```

响应：

```json
{
  "code": "0",
  "message": "OK",
  "requestId": "5faadf43-7d74-4d3d-a4e8-b233842b16a3",
  "data": {
    "userId": 10001,
    "currency": "CNY",
    "totalBalance": 1000000.00,
    "availableBalance": 820000.00,
    "frozenBalance": 180000.00,
    "positionMarketValue": 205000.00,
    "totalAsset": 1025000.00
  }
}
```

### 15.5 创建回测任务示例

请求：

```http
POST /api/backtests
Authorization: Bearer <token>
Content-Type: application/json
```

```json
{
  "strategyName": "double-ma",
  "symbol": "600519.SH",
  "startDate": "2024-01-01",
  "endDate": "2025-12-31",
  "params": {
    "shortWindow": 5,
    "longWindow": 20,
    "initialCash": 1000000
  }
}
```

响应：

```json
{
  "code": "0",
  "message": "OK",
  "requestId": "7fc31479-c7ae-4055-a44d-e15ea8982a72",
  "data": {
    "jobId": "bt-20260325-001",
    "status": "PENDING"
  }
}
```

## 16. 演示场景

- 导入股票历史行情
- 提交买入或卖出委托
- 撮合引擎输出成交结果
- 账户资金与持仓实时更新
- 风控模块生成风险快照与风险事件
- 发起一次回测任务并查看收益、回撤、夏普指标
- Grafana 展示撮合吞吐、订单延迟、Kafka Lag、风险计算耗时

## 17. 8 周开发计划

### 第 1 周

- 搭建 monorepo、`docker-compose`、基础依赖、数据库初始化
- 建立核心表结构
- 完成 `gateway-service`、`order-service`、`account-service` 骨架

### 第 2 周

- 接入行情导入流程
- 建立 `market-service`
- 接 `ClickHouse` 与 `Redis`

### 第 3 周

- 完成下单、撤单接口
- 设计 outbox 或可靠消息机制
- 接入 `Kafka`

### 第 4 周

- 开发 `C++ matching-engine`
- 实现订单簿、价格优先、时间优先、部分成交、撤单
- 打通成交回报链路

### 第 5 周

- 完成账户、持仓随成交回报联动更新
- 实现成交事件幂等消费与重放

### 第 6 周

- 开发 `C++ risk-core` 和 `Java risk-service`
- 实现回撤、集中度、仓位限制、止损阈值能力

### 第 7 周

- 开发 `backtest-service` 和 `market-replay-engine`
- 输出回测任务与结果

### 第 8 周

- 补监控、日志、审计、压测文档
- 完成 README、架构图、接口文档、演示脚本

## 18. 简历表达建议

### 18.1 一句话项目描述

设计并实现基于 `Java + C++` 的证券行情、模拟撮合与组合风控平台，完成行情接入、订单处理、撮合成交、持仓更新、风险计算、回测分析与审计监控的全链路闭环。

### 18.2 简历要点

- 基于 `Kafka + gRPC + Redis + PostgreSQL + ClickHouse` 构建事件驱动架构，实现订单与成交异步解耦、热点行情缓存和历史数据分析查询。
- 使用 `C++` 实现订单簿与模拟撮合引擎，支持价格优先时间优先、部分成交、撤单和历史行情回放。
- 使用 `Java/Spring Boot` 实现订单、账户、组合、风控、回测、审计等中后台服务。
- 设计幂等消费、失败重试、数据重放、审计日志和监控告警机制，提升系统稳定性、可恢复性和可观测性。

### 18.3 面试重点

- 为什么选择 `Java + C++` 混合架构
- 为什么撮合和实时风控核心使用 `C++`
- 为什么账户、订单、报表、审计使用 `Java`
- 为什么使用 `Kafka` 解耦订单、成交和风险事件
- 如何保证幂等性、一致性和补数重放能力
- 如何划分 `Redis`、`PostgreSQL` 和 `ClickHouse` 的职责边界

## 19. 后续文档拆分建议

当前文档可继续拆分为以下独立文档：

- `docs/architecture.md`：只保留架构图、时序图、服务职责
- `docs/api.md`：只保留接口契约与示例
- `docs/schema.md`：只保留数据库设计与 SQL
- `docs/benchmark.md`：记录压测方法和性能结果
- `docs/interview-notes.md`：整理项目亮点与面试回答

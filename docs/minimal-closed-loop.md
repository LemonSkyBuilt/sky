# 最小闭环开发文档

## 1. 文档目标

本文档用于指导当前仓库从“项目骨架”推进到“可运行、可开发、可验证”的最小交易闭环。

第一阶段只做一条最小 happy path：

1. 用户调用 `order-service` 提交限价单。
2. 订单写入 `PostgreSQL`。
3. `order-service` 通过 outbox 将订单事件投递到消息队列。
4. `matching-engine` 消费订单事件并执行最小撮合。
5. `matching-engine` 产生成交事件并写回消息队列。
6. `account-service` 消费成交事件并更新资金。
7. `portfolio-service` 消费成交事件并更新持仓。
8. 通过查询接口验证订单、账户、持仓结果。

## 2. 第一阶段范围

### 2.1 本阶段必须完成

- `deploy/docker-compose.yml`
- `sql/postgres` 初始化脚本或 `Flyway` 迁移
- `java-services/common-starter` 公共开发底座
- `java-services/order-service`
- `java-services/account-service`
- `java-services/portfolio-service`
- `cpp-services/matching-engine`
- 本地联调脚本和最小验证脚本

### 2.2 本阶段明确不做

- `gateway-service` 的完整网关能力
- `market-service`、`risk-service`、`backtest-service`、`audit-service`
- `ClickHouse`、`Grafana`、`Prometheus` 的完整接入
- 完整鉴权、RBAC、风控规则引擎
- `gRPC + Protobuf` 正式通信协议

说明：
第一阶段允许先使用 `Kafka` 兼容消息队列加 `JSON` 事件体，优先打通链路。`gRPC + Protobuf` 放到第二阶段接入，不要一开始把复杂度堆满。

## 3. 完成标准

满足以下条件，即认为最小闭环完成：

- 本地可通过 `docker compose up -d` 启动依赖环境。
- `POST /api/orders` 可以成功返回 `orderId`。
- 订单表和 outbox 表可以看到新增记录。
- `matching-engine` 可以消费订单事件并输出成交事件。
- `account-service` 可以基于成交事件扣减或增加账户余额。
- `portfolio-service` 可以基于成交事件更新持仓。
- `GET /api/orders/{orderId}`、`GET /api/accounts/{userId}/summary`、`GET /api/positions?userId=...` 可以看到链路结果。
- 至少有一条自动化验证路径：集成测试、脚本回放、或端到端 smoke test。

## 4. 技术落地约定

为降低第一阶段的不确定性，建议直接固定以下选择：

- Java 持久化：`Spring JDBC` 或 `JdbcTemplate`
- Java 数据库迁移：`Flyway`
- Java 消息：`Spring Kafka`
- C++ 消息客户端：`librdkafka`
- 本地消息队列：优先 `Redpanda`，因为兼容 `Kafka` 协议且本地启动简单
- 数据库：`PostgreSQL`
- 事件序列化：第一阶段使用 `JSON`

说明：
不建议第一阶段上 `JPA`。订单、成交、账户、持仓这些核心表后续会涉及显式 SQL、幂等更新和事务控制，直接使用 `JdbcTemplate` 更可控。

## 5. 分步骤开发计划

### Step 0. 锁定目录和分支策略

目标：先把目录结构和开发边界固定，避免后面反复改。

建议新增目录：

```text
deploy/
  docker-compose.yml
sql/
  postgres/
scripts/
  dev-up.ps1
  dev-down.ps1
  mock-order-flow.ps1
docs/
  minimal-closed-loop.md
```

建议分支策略：

- `feat/minimal-closed-loop-infra`
- `feat/order-service-minimal-api`
- `feat/matching-engine-v1`
- `feat/account-and-portfolio-consumers`

完成标准：

- 目录结构确定
- 第一阶段只围绕最小闭环开发，不再继续平铺更多服务

### Step 1. 先把本地依赖跑起来

目标：先解决“服务跑不起来”的问题，再写业务代码。

需要新增：

- `deploy/docker-compose.yml`
- 可选 `deploy/.env.example`

依赖建议：

- `PostgreSQL`
- `Redpanda`
- 可选 `Redpanda Console`

本阶段先不要引入太多组件。`Redis` 不是最小闭环的阻塞项，可以第二批再加。

建议验证命令：

```bash
docker compose -f deploy/docker-compose.yml up -d
docker compose -f deploy/docker-compose.yml ps
```

完成标准：

- 本地数据库和消息队列都可启动
- Java 服务和 C++ 服务都能连上这两个依赖

### Step 2. 建立最小数据库模型

目标：先把订单链路所需的持久化模型落地。

建议新增：

- `sql/postgres/V1__init_minimal_loop.sql`

第一阶段最少需要以下表：

- `stock_order`
- `stock_trade`
- `cash_account`
- `position`
- `event_outbox`

关键约束建议：

- `stock_order(order_id)` 唯一
- `stock_order(user_id, request_id)` 唯一，用于幂等
- `stock_trade(trade_id)` 唯一
- `cash_account(user_id, currency_code)` 唯一
- `position(user_id, symbol)` 唯一
- `event_outbox(event_id)` 唯一

同时在 `order-service`、`account-service`、`portfolio-service` 中接入 `Flyway`。

完成标准：

- 本地启动服务时自动执行迁移
- 核心表结构可用

### Step 3. 补齐 Java 公共开发底座

目标：避免每个服务都从零写 Web 基建。

建议在 `java-services/common-starter` 中增加：

- 统一响应对象 `ApiResponse`
- 全局异常处理 `GlobalExceptionHandler`
- 业务异常基类 `BusinessException`
- 请求 `requestId` 过滤器或拦截器
- 基础日志上下文
- 公共 Jackson 配置

建议补充的依赖：

- `spring-boot-starter-jdbc`
- `flyway-core`
- `postgresql`
- `spring-kafka`

说明：
公共能力要轻，不要把所有业务逻辑塞进 `common-starter`。这里只放通用技术底座。

完成标准：

- 三个 Java 服务可以复用同一套响应、异常和基础配置

### Step 4. 实现 `order-service` 最小 API

目标：先把订单入口做出来。

必须提供的接口：

- `POST /api/orders`
- `GET /api/orders/{orderId}`

推荐包结构：

```text
controller/
dto/
service/
repository/
domain/
```

`POST /api/orders` 第一版只支持：

- `LIMIT` 单
- `BUY` / `SELL`
- 单用户单市场场景

下单处理流程：

1. 校验请求参数。
2. 生成 `orderId`。
3. 在一个本地事务中同时写入 `stock_order` 和 `event_outbox`。
4. 返回 `NEW` 状态。

建议最小请求字段：

- `requestId`
- `userId`
- `symbol`
- `side`
- `orderType`
- `price`
- `quantity`

完成标准：

- 可通过 HTTP 成功创建订单
- 表中可看到订单记录和待发送 outbox 记录

### Step 5. 在 `order-service` 中实现 outbox relay

目标：把数据库里的待发送事件真正发到消息队列。

建议做法：

- 在 `order-service` 内增加一个定时任务或后台轮询任务
- 周期性扫描 `event_outbox` 中 `NEW` 状态的数据
- 投递到主题 `orders.created`
- 投递成功后将 outbox 状态更新为 `SENT`

第一阶段建议只定义一种订单事件：

```json
{
  "eventId": "evt-20260414-0001",
  "orderId": "ord-20260414-0001",
  "requestId": "req-20260414-0001",
  "userId": 10001,
  "symbol": "600519.SH",
  "side": "BUY",
  "orderType": "LIMIT",
  "price": "1688.88",
  "quantity": 100,
  "occurredAt": "2026-04-14T21:30:00+08:00"
}
```

第一阶段不要过早优化：

- 不要先做复杂分片策略
- 不要先做死信队列
- 不要先做多 topic 编排

完成标准：

- 提交订单后，`orders.created` 主题可以看到消息

### Step 6. 把 `matching-engine` 从骨架改成真实服务

目标：让当前的 C++ 骨架真正承担最小撮合能力。

当前 `cpp-services/matching-engine/src/main.cpp` 还只是启动和自检骨架，下一步建议把代码拆成以下结构：

```text
src/
  main.cpp
  app.cpp
  order_book.cpp
  order_event_consumer.cpp
  trade_event_producer.cpp
include/
  trading/matching/
    app.h
    order_book.h
    model.h
```

第一阶段最小能力要求：

- 消费 `orders.created`
- 按 `symbol` 维护内存订单簿
- 支持价格优先、时间优先
- 支持完全成交和部分成交
- 成交后发送 `trades.created`

建议第一阶段不做：

- 撤单
- 多市场分片
- 持久化订单簿
- 高频性能优化

建议成交事件格式：

```json
{
  "eventId": "evt-20260414-1001",
  "tradeId": "trd-20260414-0001",
  "orderId": "ord-20260414-0001",
  "userId": 10001,
  "symbol": "600519.SH",
  "side": "BUY",
  "price": "1688.88",
  "quantity": 100,
  "tradeAmount": "168888.00",
  "occurredAt": "2026-04-14T21:31:02+08:00"
}
```

完成标准：

- `matching-engine` 启动后可以持续消费订单事件
- 产生的成交事件能写入 `trades.created`

### Step 7. 实现 `account-service` 成交消费与资金更新

目标：成交发生后，账户资金能发生真实变化。

建议提供：

- 成交事件消费者
- 账户查询接口 `GET /api/accounts/{userId}/summary`

最小处理规则：

- 买入成交：减少 `available_balance` 和 `total_balance`
- 卖出成交：增加 `available_balance` 和 `total_balance`
- 使用 `trade_id` 做幂等去重

注意：
如果后续要拆成“下单先冻结、成交后扣减冻结”的真实模型，第一阶段也可以先简化为直接扣减余额，先保证链路跑通。

完成标准：

- `trades.created` 到来后，资金表发生预期变化
- 同一笔成交重复消费不会重复记账

### Step 8. 实现 `portfolio-service` 成交消费与持仓更新

目标：成交发生后，用户持仓能同步变化。

建议提供：

- 成交事件消费者
- 持仓查询接口 `GET /api/positions?userId=...`

最小处理规则：

- 买入成交：增加 `total_quantity`，更新 `avg_cost`
- 卖出成交：减少 `total_quantity`
- 使用 `trade_id` 做幂等去重

第一阶段允许简化：

- 不先计算复杂浮盈亏
- 不先做 T+1、可卖数量冻结等真实交易细节

完成标准：

- 查询接口能看到持仓数量变化
- 重复消费不会造成持仓重复累计

### Step 9. 补联调脚本和端到端验收脚本

目标：让最小闭环可以被稳定复现，而不是只能手工点点点。

建议新增：

- `scripts/dev-up.ps1`
- `scripts/dev-down.ps1`
- `scripts/mock-order-flow.ps1`

脚本职责建议：

- `dev-up.ps1`：启动依赖环境
- `dev-down.ps1`：停止依赖环境
- `mock-order-flow.ps1`：创建测试账户、提交订单、轮询订单和账户结果

最小验收用例建议：

1. 初始化一个余额充足的测试账户。
2. 提交一笔买单。
3. 等待撮合和成交事件传播。
4. 查询订单状态。
5. 查询账户余额。
6. 查询持仓结果。

完成标准：

- 至少可以通过一条脚本重复跑通最小闭环

### Step 10. 补测试和基础可观测性

目标：让当前闭环可维护，而不是只跑一次。

至少要补：

- `order-service` API 集成测试
- outbox relay 测试
- `account-service` 成交幂等测试
- `portfolio-service` 成交幂等测试
- `matching-engine` 订单簿单元测试

最小监控建议：

- Java 服务暴露 `health` 和 `info`
- 记录 `requestId`、`orderId`、`tradeId`
- `matching-engine` 启动时打印订阅主题和处理统计

完成标准：

- 关键模块至少有一层自动化验证
- 问题出现时可以根据日志追踪到订单和成交

## 6. 推荐的开发顺序

如果按一周左右做第一版，建议顺序如下：

### Day 1

- 完成 `deploy/docker-compose.yml`
- 完成 `sql/postgres/V1__init_minimal_loop.sql`
- 完成 `common-starter` 的基础设施依赖

### Day 2

- 完成 `order-service` 的下单接口
- 完成订单表和 outbox 表写入
- 完成 `GET /api/orders/{orderId}`

### Day 3

- 完成 outbox relay
- 本地确认 `orders.created` 已有消息

### Day 4

- 完成 `matching-engine` 最小订单簿
- 输出 `trades.created`

### Day 5

- 完成 `account-service` 成交消费和查询接口
- 完成 `portfolio-service` 成交消费和查询接口

### Day 6

- 补联调脚本
- 跑通端到端 smoke test

### Day 7

- 补最关键的自动化测试
- 修正幂等、日志、异常处理问题

## 7. 每一步的提交边界建议

为了避免一次提交过大，建议按以下粒度提交：

- `feat: add local infra for minimal closed loop`
- `feat: add minimal postgres schema and flyway baseline`
- `feat: implement order creation and outbox relay`
- `feat: implement matching engine order intake and trade emission`
- `feat: consume trade events in account and portfolio services`
- `docs: add minimal closed loop development guide`

## 8. 第二阶段衔接方向

最小闭环完成后，再继续做以下内容：

- 引入 `Redis` 缓存热点账户或行情快照
- 将 `JSON` 事件迁移为 `Protobuf`
- 为 Java 与 C++ 增加 `gRPC` 控制面接口
- 补撤单、部分成交、冻结资金、可卖数量等真实交易细节
- 接入 `risk-service`
- 接入 `market-service` 和 `market-replay-engine`

## 9. 一句话执行原则

先打通一条真实可验证的交易链路，再扩功能；先保证链路成立，再优化架构细节。

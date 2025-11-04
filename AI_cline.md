
## install VSCode

## install Plugin

- clangd (LSP for C/C++)
- Bifrost (LSP to MCP)
- cline

## configure cline

### configure model

[aliyun](https://help.aliyun.com/zh/model-studio/new-free-quota?spm=a2ty02.33053938.0.0.27e674a12SXemq)

- API Provider: OpenAI Compatible
- Base URL: [aliyun](https://dashscope.aliyuncs.com/compatible-mode/v1)
- API key
- Model ID: qwen3-max

### configure MCP servers

- Fetch (in MarketPlace)
- Context7 (in MarketPlace)
- Bifrost (Manual Add)

```json
{
  "mcpServers": {
    "github.com/zcaceres/fetch-mcp": {
      "command": "node",
      "args": ["/home/user/Cline/MCP/fetch-mcp/dist/index.js"],
      "env": {
        "DEFAULT_LIMIT": "50000"
      },
      "disabled": false,
      "autoApprove": []
    },
    "github.com/upstash/context7-mcp": {
      "command": "npx",
      "args": ["-y", "@upstash/context7-mcp"],
      "disabled": false,
      "autoApprove": []
    },
    "Bifrost": {
      "command": "npx",
      "args": ["-y", "supergateway", "--sse", "http://localhost:8008/sse"],
      "disabled": false,
      "autoApprove": [],
      "timeout": 600
    }
  }
}
```

## Cline Tips

[cline-tips](https://github.com/dorukyy/cline-tips)
[cline-bot](https://docs.cline.bot/getting-started/understanding-context-management)

[clinerules](https://docs.cline.bot/features/cline-rules)

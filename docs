# BigBrother Future Enhancements

## Overview
Evolution from activity tracking to **work documentation and billing tool** for consultancy work.

## Core Goals
- Track activity in relation to consultancy work
- Identify which customer/project work was done for
- Export data in formats suitable for LLMs and reporting
- Capture richer context for better work summaries
- Support git-heavy development workflows

---

## 1. Enhanced Context Capture

### Git Repository Integration
- **Capture git branch names** - Know which feature/ticket you worked on
- **Track commits made during sessions** - What you actually completed
- **Monitor git status** - Files changed, additions/deletions
- **Branch switching events** - When you moved between tasks
- **PR/issue numbers** from branch names (e.g., `feature/PROJ-123-add-auth`)

### File System Intelligence  
- **Working directory tracking** - Not just window titles, but CWD of terminal/IDE
- **File edit timestamps** - Which specific files you modified
- **Project/workspace detection** - Auto-detect VS Code workspace, git root
- **File type categorization** - Code vs docs vs config vs tests

### Application-Specific Data
- **Browser tab URLs** - Not just titles, but actual URLs for research tracking
- **IDE project context** - Which workspace/solution is open
- **Terminal command history** - Commands executed (if feasible/privacy-acceptable)
- **Database connections** - Which databases/servers you worked with

---

## 2. Customer/Project Tagging System

### Automatic Inference
```
Project Detection Rules:
- Path contains "client_acme" → Tag: ACME Corp
- Git remote = github.com/bigcorp → Tag: BigCorp  
- Browser: jira.company.com/PROJECT-* → Tag: Project X
- File path: ~/work/consulting/client_name → Tag: client_name
```

### Manual Override UI
- **Tag sessions manually** - "This session was for Client X"
- **Project selector** - Dropdown to assign work to projects
- **Time allocation** - Split session between multiple clients
- **Billable flag** - Mark sessions as billable/non-billable

### Smart Rules Engine
```json
{
  "rules": [
    {
      "customer": "ACME Corp",
      "patterns": [
        {"type": "path", "contains": "/acme/"},
        {"type": "git_remote", "matches": "github.com/acme/*"},
        {"type": "browser", "domain": "acme-jira.atlassian.net"}
      ]
    }
  ]
}
```

---

## 3. Export Formats for LLM & Reporting

### Structured Exports

#### For LLMs (Markdown)
```markdown
# Work Session Report: Monday, September 30, 2025

## Session 1: 18:18-18:41 (27 minutes)
**Project:** ACME Corp - Authentication Service
**Branch:** feature/ACME-123-oauth-integration

### Activities:
- **Code Development** (15m)
  - Modified: `auth_service.py`, `oauth_handler.py`
  - Branch: feature/ACME-123-oauth-integration
  - Files edited: 5 files, ~200 lines changed

- **Research & Documentation** (8m)
  - OAuth 2.0 specification
  - dbt Labs blog articles
  - Stack Overflow: OAuth token refresh

- **Commits Made:**
  - 18:25 - "Add OAuth provider configuration"
  - 18:38 - "Implement token refresh logic"

### Context:
Working on implementing OAuth 2.0 for ACME's authentication service.
Researched token refresh patterns and implemented provider configuration.
```

#### For Billing (CSV)
```csv
Date,Start,End,Duration,Project,Billable,Description
2025-09-30,18:18,18:41,0.45,ACME Corp,Yes,"OAuth implementation - auth_service feature"
```

#### For Analysis (JSON)
```json
{
  "session": {
    "date": "2025-09-30",
    "duration": 1620,
    "project": "ACME Corp",
    "activities": [
      {
        "type": "coding",
        "duration": 900,
        "files": ["auth_service.py", "oauth_handler.py"],
        "git_branch": "feature/ACME-123",
        "commits": 2
      },
      {
        "type": "research", 
        "duration": 480,
        "topics": ["OAuth 2.0", "dbt Labs", "token refresh"]
      }
    ]
  }
}
```

---

## 4. LLM Integration Features

### Direct AI Summarization
- **Built-in "Summarize" button** - Send to OpenAI/Claude API
- **Daily summary generation** - Auto-generate EOD reports
- **Weekly summaries** - For status meetings
- **Template prompts** - "Write timesheet entry", "Write standup update", "Generate invoice description"

### Context Enrichment for LLMs
```
Include in LLM prompt:
- Session timeline (what/when)
- Git commits with messages
- Files modified (grouped by module)
- Research topics (from browser)
- Time spent per activity type
- Project/customer context
```

### Export Templates
```
Template: "Invoice Description"
Prompt: "Based on this session data, write a 2-3 sentence professional 
description suitable for a consulting invoice line item."

Template: "Daily Standup"
Prompt: "Summarize what I worked on today in standup format: 
What I did, what I'm doing next, any blockers."
```

---

## 5. Git-Specific Enhancements

### Monitor Git Events
- **Branch changes** - When you switch branches
- **Commits** - Timestamp, message, files changed
- **Stashes** - When you context-switch
- **Pull/Push** - Collaboration points
- **Merge conflicts** - Time spent resolving

### Git Activity Timeline
```
Session 1: feature/ACME-123-oauth
├─ 18:18 - Checkout branch
├─ 18:25 - Commit: "Add OAuth config" (+50, -10)
├─ 18:30 - Switch to main (stash changes)
├─ 18:32 - Switch back to feature/ACME-123
└─ 18:38 - Commit: "Token refresh" (+120, -5)
```

### Code Metrics
- **Lines changed** - Productivity indicator
- **Files touched** - Scope of work
- **Languages used** - Python, TypeScript, SQL, etc.
- **Test files** - Time spent on testing

---

## 6. Work Session Intelligence

### Automatic Activity Classification
```
Categories:
- Deep Work (coding, long focus periods)
- Research (documentation, Stack Overflow, docs)
- Communication (email, Slack, Teams)
- Meetings (Zoom, Teams)
- Context Switching (rapid app changes)
- Debugging (console, logs, debugger)
```

### Productivity Metrics
- **Focus time** - Uninterrupted work periods
- **Interruption count** - How often you switched
- **Deep work percentage** - Time in flow state
- **Context switch cost** - Time lost to switching

---

## 7. Viewer Enhancements for Consultancy

### Project Dashboard View
```
Projects:
├─ ACME Corp (25h this week)
│  ├─ OAuth Integration (15h)
│  └─ Bug Fixes (10h)
├─ BigCorp (12h this week)
│  └─ Database Migration (12h)
```

### Filtering & Search
- Filter by customer/project
- Search by file name, git branch, window title
- Date range selection
- Export filtered results

### Time Allocation View
```
This Week:
ACME Corp:    ████████████████░░░░  40h (70%)
BigCorp:      ████████░░░░░░░░░░░░  15h (26%)
Internal:     ██░░░░░░░░░░░░░░░░░░   2h  (4%)
```

---

## 8. Privacy & Security Considerations

- **Sensitive data filtering** - Redact passwords, tokens from window titles
- **Customer data separation** - Keep client data isolated
- **Encrypted storage** - Protect client information
- **Data retention policies** - Auto-delete old sessions
- **Selective export** - Choose what to include in exports

---

## 9. Integration Ideas

### External Tools
- **Jira/Linear integration** - Link to tickets automatically
- **Toggl/Harvest export** - Push time entries
- **Calendar integration** - Mark meeting times
- **Slack status** - Auto-update based on activity
- **GitHub API** - Enrich with PR data

### Automation
- **End-of-day automation** - Generate summary at 5pm
- **Weekly report email** - Auto-send to yourself Friday
- **Invoice generation** - Export to invoicing tools
- **Backup to cloud** - S3, Dropbox, etc.

---

## Implementation Roadmap

### Phase 1 (High Priority)
1. **Git integration** - Branch + commit tracking (biggest value for dev workflow)
2. **Path-based project detection** - Auto-tag sessions to customers
3. **Markdown export** - Simple, LLM-friendly format
4. **Browser URL capture** - Better context than just titles
5. **Manual project tagging UI** - Override automatic detection

### Phase 2 (Medium Priority)
6. **Direct LLM summarization** - Built-in AI summary button
7. **CSV export for billing** - Timesheet/invoice ready
8. **Activity classification** - Coding vs research vs meetings
9. **Project dashboard view** - High-level time allocation
10. **Filter & search functionality** - Find specific work sessions

### Phase 3 (Nice to Have)
11. **Productivity metrics** - Focus time, interruptions, deep work
12. **External tool integrations** - Jira, Toggl, GitHub
13. **Automated reporting** - Daily/weekly summaries
14. **Advanced git metrics** - Lines changed, code quality
15. **Time allocation splitting** - Multiple projects per session

---

## Technical Considerations

### Data Structure Changes Needed
- Add `project_id` / `customer_tag` to sessions
- Add `git_branch`, `git_commits` to focus events
- Add `url` field for browser windows
- Add `working_directory` to capture context
- Add `activity_type` classification field

### New Components Required
- **Git monitor module** - Hook into git events
- **URL extractor** - For browser applications
- **Project detection engine** - Rule-based tagging
- **Export engine** - Multiple format support
- **LLM integration module** - API calls to AI services

### Performance Considerations
- Git monitoring should be lightweight
- URL extraction needs browser-specific handling
- Export should handle large date ranges efficiently
- LLM calls should be async with progress indicators

---

## Notes
- Keep backward compatibility with existing JSON format
- All new features should be optional/configurable
- Privacy-first approach for sensitive data
- Design for offline-first, sync later if cloud features added

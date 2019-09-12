<h1 class="contract">store</h1>

---
spec_version: "0.2.0"
title: Store Tokens
summary: 'Store {{nowrap value}}'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{account}} agrees to store {{value}}, causing these specified tokens to be non-transferable, and only transferable upon waiting 3 days after executing a withdraw action.













<h1 class="contract">withdraw</h1>

---
spec_version: "0.2.0"
title: Withdraw Tokens
summary: 'Withdraw {{nowrap value}}'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{account}} agrees to withdraw {{value}}, which will become transferable 3 days after this action. The withdraw process will be reset to 3 days for any tokens currently in withdraw at the time of this action.  














<h1 class="contract">airgrab2</h1>

---
spec_version: "0.2.0"
title: Airgrab Tokens
summary: 'Airgrab {{nowrap amount}}'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{account}} agrees to airgrab {{amount}}, this is not an investment contract nor does it represent any type of claim, loan or ownership interest.  Airgrabbing SOV tokens does not provide airgrabber with rights of any form, including, but not limited to, any voting, or other financial or legal rights.













<h1 class="contract">airburn</h1>

---
spec_version: "0.2.0"
title: Airburn Tokens
summary: 'Airburn {{nowrap amount}}'
icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{account}} agrees to burn {{amount}} from the balance of sovmintofeos.     












<h1 class="contract">transfer</h1>

spec_version: "0.2.0"
title: Transfer Tokens
summary: 'Send {{nowrap quantity}} from {{nowrap from}} to {{nowrap to}}'
icon: @ICON_BASE_URL@/@TRANSFER_ICON_URI@
---

{{from}} agrees to send {{quantity}} to {{to}}. {{from}} agrees to burn a percent of this transaction corresponding to burn schedule.

{{#if memo}}There is a memo attached to the transfer stating:
{{memo}}
{{/if}}

If {{from}} is not already the RAM payer of their {{asset_to_symbol_code quantity}} token balance, {{from}} will be designated as such. As a result, RAM will be deducted from {{from}}’s resources to refund the original RAM payer.

If {{to}} does not have a balance for {{asset_to_symbol_code quantity}}, {{from}} will be designated as the RAM payer of the {{asset_to_symbol_code quantity}} token balance for {{to}}. As a result, RAM will be deducted from {{from}}’s resources to create the necessary records. 